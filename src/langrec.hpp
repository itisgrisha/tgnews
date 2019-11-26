#include <unordered_map>
#include <iostream>
#include <string>
#include <vector>
#include <thread>

#include "cld3/nnet_language_identifier.h"
#include "html_parse.hpp"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

using lang2file_t = std::unordered_map<std::string,std::vector<std::string>>;


void RecogTask(std::vector<HTMLDocument>* documents, chrome_lang_id::NNetLanguageIdentifier& langrec, size_t begin, size_t end) {
    for (size_t i = begin; i < end; ++i) {
        auto& html_doc = documents->at(i);
        auto langrec_result = langrec.FindLanguage(html_doc.GetText());
        html_doc.SetMeta("lang", langrec_result.language);
        html_doc.SetMeta("lang_score", std::to_string(langrec_result.probability));
    }
}


void RecognizeLanguage(std::vector<HTMLDocument>* documents) {
    chrome_lang_id::NNetLanguageIdentifier langrec(0, 1000);
    size_t step = documents->size() / 4;
    size_t begin = 0;
    size_t end = step;
    std::vector<std::thread> workers;
    for (; begin + step < documents->size(); begin+=step) {
        end = std::min(documents->size(), begin+step);
        workers.emplace_back(RecogTask, documents, std::ref(langrec), begin, end);
    }
    end = std::min(documents->size(), begin+step);
    if (begin < end) {
        workers.emplace_back(RecogTask, documents, std::ref(langrec), begin, end);
    }

    for (auto& worker : workers) {
        worker.join();
    }
}

std::vector<HTMLDocument> GetRuEnDocs(std::vector<HTMLDocument>* docs) {
    std::vector<HTMLDocument> ruen_docs;
    for (auto& doc : *docs) {
        auto lang = doc.GetMeta("lang");
        if (lang == "ru" || lang == "en") {
            ruen_docs.emplace_back(std::move(doc));
        }
    }
    return ruen_docs;
}
