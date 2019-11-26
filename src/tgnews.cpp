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


int main(int argc, char *argv[]) {
    std::string task(argv[1]);
    std::string input_folder(argv[2]);

    auto html_files = GlobHTML(input_folder);

    auto start = std::chrono::high_resolution_clock::now();
    auto docs = LoadDocs(html_files);
    std::cout << "LOAD DOCS: " << std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now()-start).count() << std::endl;
    start = std::chrono::high_resolution_clock::now();

    if (task == "languages") {
        RecognizeLanguage(&docs);
        DumpLanguage(docs, input_folder);
    } else if (task == "news") {
    } else if (task == "dump") {
        RecognizeLanguage(&docs);
        MakeTsv(docs, argv[3]);
    } else if (task == "features") {
        RecognizeLanguage(&docs);
        std::cout << "REC LANG: " << std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now()-start).count()<< std::endl;
        start = std::chrono::high_resolution_clock::now();
        GenerateFeatures(docs);
        std::cout << "FEATURES GEN: " << std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now()-start).count() << std::endl;
        start = std::chrono::high_resolution_clock::now();
    }
    
    return 0;
}
