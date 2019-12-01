#include <unordered_map>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <initializer_list>

#include "cld3/nnet_language_identifier.h"
#include "html_parse.hpp"
#include "common.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

using lang2file_t = std::unordered_map<std::string,std::vector<std::string>>;


std::unordered_map<std::string, std::string> GetUrl2Lang() {
    std::unordered_map<std::string, std::string> url2lang;
    std::ifstream input;
    std::string url;
    for (const std::string& lang_name : {"ru", "en", "other"}) {
        input.open(std::string("meta/site_").append(lang_name).append("_lang.txt"));
        while (std::getline(input, url)) {
            url2lang.emplace(url, lang_name);
        }
        input.close();
    }
    return url2lang;
}


void RecogTask(std::vector<HTMLDocument>* documents,
               const std::unordered_map<std::string, std::string>& site2lang,
               chrome_lang_id::NNetLanguageIdentifier& langrec,
               bool use_lists,
               size_t begin,
               size_t end) {
    for (size_t i = begin; i < end; ++i) {
        auto& html_doc = documents->at(i);
        auto it = site2lang.find(html_doc.GetMeta("og:site_name"));
        if (it != site2lang.end() && use_lists) {
            html_doc.SetMeta("lang", it->second);
            html_doc.SetMeta("lang_score", "1");
        } else {
            auto langrec_result = langrec.FindLanguage(html_doc.GetText());
            html_doc.SetMeta("lang", langrec_result.language);
            html_doc.SetMeta("lang_score", std::to_string(langrec_result.probability));
        }
    }
}


std::vector<HTMLDocument> RecognizeLanguage(std::vector<HTMLDocument>* documents, bool use_lists=true) {
    auto site2lang = GetUrl2Lang();
    chrome_lang_id::NNetLanguageIdentifier langrec(0, 1000);
    size_t step = std::max<size_t> (1u, documents->size() / kNumThreads);
    size_t begin = 0;
    size_t end = step;
    std::vector<std::thread> workers;
    for (; begin < documents->size(); begin+=step) {
        end = std::min(documents->size(), begin+step);
        workers.emplace_back(RecogTask, documents, std::cref(site2lang), std::ref(langrec), use_lists, begin, end);
    }
    for (auto& worker : workers) {
        worker.join();
    }
    std::vector<HTMLDocument> recognized;
    for (auto& doc : *documents) {
        auto doc_lang = doc.GetMeta("lang");
        if (doc_lang == "ru" || doc_lang == "en") {
            recognized.emplace_back(std::move(doc));
        }
    }
    return recognized;
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
