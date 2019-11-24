#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <memory>

#include "udpipe.h"
#include "html_parse.hpp"

namespace udpipe = ufal::udpipe;


void TokenizeDocuments(std::vector<HTMLDocument>* documents, const std::string& model_path) {
    std::unique_ptr<udpipe::model> model;
    model.reset(udpipe::model::load(model_path.c_str()));

    std::unique_ptr<udpipe::input_format> reader;
    reader.reset(model->new_tokenizer("tokenizer"));

    std::unique_ptr<udpipe::output_format> writer(udpipe::output_format::new_output_format("conllu"));
    auto sentence = udpipe::sentence();
    std::string error;
    for (auto& doc : *documents) {
        if (doc.GetMeta("lang") != "ru") {
            continue;
        }
        reader->reset_document(doc.GetMeta("path"));
        reader->set_text(doc.GetTextP());
        while (reader->next_sentence(sentence, error)) {
            model->tag(sentence, udpipe::pipeline::DEFAULT, error);
            model->parse(sentence, udpipe::pipeline::DEFAULT, error);
            writer->write_sentence(sentence, std::cout);
        }
    }
}
