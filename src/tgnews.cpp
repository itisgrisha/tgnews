#include <iostream>
#include <string>
#include <ctime>
#include <chrono>

#include "utils.hpp"
#include "html_parse.hpp"
#include "debug.hpp"
#include "langrec.hpp"
#include "features.hpp"
#include "io.hpp"
#include "news_det.hpp"


int main(int argc, char *argv[]) {
    std::string task(argv[1]);
    std::string input_folder(argv[2]);

    auto html_files = GlobHTML(input_folder);

    auto docs = LoadDocs(html_files);

    if (task == "languages") {
        RecognizeLanguage(&docs);
        DumpLanguage(docs, input_folder);
    } else if (task == "news") {
        RecognizeLanguage(&docs);
        auto features = GenerateFeatures(docs, 3);
        DetectNews(&features);
        DumpNewsDet(features, input_folder);
    } else if (task == "news_test") {
        RecognizeLanguage(&docs);
        auto features = GenerateFeatures(docs, 3);
        DetectNews(&features);
        DumpNewsDetTest(features, input_folder);
    } else if (task == "dump") {
        RecognizeLanguage(&docs);
        MakeTsv(docs, argv[3]);
    } else if (task == "features") {
        auto start = std::chrono::high_resolution_clock::now();
        RecognizeLanguage(&docs);
        std::cout << "REC LANG: " << std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now()-start).count()<< std::endl;
        start = std::chrono::high_resolution_clock::now();
        GenerateFeatures(docs, 3);
        std::cout << "FEATURES GEN: " << std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now()-start).count() << std::endl;
        start = std::chrono::high_resolution_clock::now();
    }
    
    return 0;
}
