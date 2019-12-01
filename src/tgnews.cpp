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
#include "news_categories.hpp"


int main(int argc, char *argv[]) {
    std::string task(argv[1]);
    std::string input_folder(argv[2]);
    auto html_files = GlobHTML(input_folder);
    auto docs = LoadDocs(html_files);
    if (task == "languages") {
        auto langs_ruen = RecognizeLanguage(&docs);
        DumpLanguage(langs_ruen, input_folder);
    } else if (task == "news") {
        auto langs_ruen = RecognizeLanguage(&docs);
        auto features = GenerateFeatures(langs_ruen, -1);
        auto news = DetectNews(&features);
        DumpNewsDet(news, input_folder);
    } else if (task == "categories") {
        auto langs_ruen = RecognizeLanguage(&docs);
        auto features = GenerateFeatures(langs_ruen, -1);
        auto news = DetectNews(&features);
        auto categories = GetCategories(news);
        DumpNewsCat(categories, input_folder);
    } else if (task == "news_test") {
        RecognizeLanguage(&docs);
        auto features = GenerateFeatures(docs, -1);
        auto news = DetectNews(&features);
        DumpNewsDetTest(news, input_folder);
    } else if (task == "dump") {
        RecognizeLanguage(&docs, true);
        auto features = GenerateFeatures(docs, -1);
        DetectNews(&features);
        MakeTsv(docs, features, argv[3]);
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
