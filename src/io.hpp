#pragma once
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>

#include <nlohmann/json.hpp>

#include "common.h"


using json = nlohmann::json;

std::vector<HTMLDocument> LoadDocs(const std::vector<std::string>& files) {
    std::vector<HTMLDocument> documents;
    for (const auto& file : files) {
        documents.emplace_back(file);
        documents.back().SetMeta("path", file);
    }
    return documents;
}


void DumpLanguage(const std::vector<HTMLDocument>& docs, const std::string& path_prefix) {
    std::unordered_map<std::string, json> lang2json;
    lang2json["ru"]["lang_code"] = "ru";
    lang2json["en"]["lang_code"] = "en";
    size_t prefix_len = path_prefix.size() + (path_prefix.back() != '/');
    for (const auto& doc : docs) {
        auto lang = doc.GetMeta("lang");
        if (lang == "ru" || lang == "en") {
            lang2json[lang]["articles"].emplace_back(doc.GetMeta("path").substr(prefix_len));
        }
    }
    json result = {std::move(lang2json["ru"]), std::move(lang2json["en"])};
    std::cout << std::setw(2) << result << std::endl;
}

void DumpNewsDet(const std::vector<DocFeatures>& docs, const std::string& path_prefix) {
    json result = {{"articles", json::array()}};
    size_t prefix_len = path_prefix.size() + (path_prefix.back() != '/');
    for (const auto& doc : docs) {
        // DIRTY
        if (doc.is_news_ && doc.lang_ == "ru") {
            result["articles"].emplace_back(doc.doc_name_.substr(prefix_len));
        }
    }
    std::cout << std::setw(2) << result << std::endl;
}

std::shared_ptr<BOWDict> ReadFeaturesNames(const std::string& path) {
    auto features_names = std::make_shared<BOWDict>();
    std::ifstream input(path);
    std::string feature;
    for (size_t feature_pos = 0; input >> feature; ++feature_pos) {
        features_names->emplace(feature, feature_pos);
    }
    return features_names;
}
