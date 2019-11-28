import sys
import csv
import pandas as pd

from tqdm import tqdm
from multiprocessing import Pool
from ufal.udpipe import Model, Sentence, ProcessingError, Pipeline, OutputFormat

if __name__ == "__main__":
    
    model_path, load_path, dump_path, n_jobs, column_id = sys.argv[1:6]
    columns = sys.argv[6:]
    
    udpipe_model = Model.load(model_path)
    tokenizer = udpipe_model.newTokenizer("tokenizer").newGenericTokenizerInputFormat()
    
    df = pd.read_csv(load_path, sep='\t', usecols=[column_id]+columns, quoting=csv.QUOTE_NONE)
    df = df[[column_id]+columns]
    
    def preproc(row):
        
        def preproc_item(text):
            if pd.isna(text):
                text = ''
            tokenizer.resetDocument()
            try:
                tokenizer.setText(text)
            except TypeError:
                print(row, text)
                1/0
            
            sentence = Sentence()
            error = ProcessingError()
            
            text = ''
            while (tokenizer.nextSentence(sentence, error)):
    
                udpipe_model.tag(sentence, Pipeline.DEFAULT, error)
                #udpipe_model.parse(sentence, Pipeline.DEFAULT, error)

                text += OutputFormat.newConlluOutputFormat().writeSentence(sentence)
                
            return text
        
        return [row[0]] + [preproc_item(text) for text in row[1:]]
        
    rows = df.values
    with Pool(int(n_jobs)) as pool:
        rows = pool.map(preproc, tqdm(rows))
        
    pd.DataFrame(rows, columns=[column_id]+columns).to_csv(dump_path, index=False)
