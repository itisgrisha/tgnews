#include <fstream>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <algorithm>

#include "common.h"
#include "html_parse.hpp"
#include "io.hpp"


void MakeTsv(std::vector<HTMLDocument>& documents,
             const std::vector<DocFeatures>& features,
             const std::string& output_fname) {
    std::unordered_set<std::string> keys;
    for (const auto& doc : documents) {
        for (const auto& key : doc.GetMetaKeys()) {
            keys.emplace(key);
        }
    }
    keys.emplace("is_news");
    keys.emplace("is_news_score");
    std::unordered_map<std::string, std::vector<std::string>> result;
    size_t total = 0;
    for (size_t i = 0; i < documents.size(); ++i) {
        auto& doc = documents.at(i);
        auto& feature = features.at(i);
        doc.SetMeta("is_news", std::to_string(feature.is_news_));
        doc.SetMeta("is_news_score", std::to_string(feature.is_news_score));
        std::string links;
        for (const auto& link : doc.GetLinks()) {
            links += link + ","; 
        }
        if (links.size() > 0) {
            links.pop_back();
        }
        links.erase(std::remove(links.begin(), links.end(), '\t'), links.end()); 
        links.erase(std::remove(links.begin(), links.end(), '\n'), links.end()); 
        result["related_links"].emplace_back(links);
        std::string text = doc.GetText();
        text.erase(std::remove(text.begin(), text.end(), '\t'), text.end()); 
        text.erase(std::remove(text.begin(), text.end(), '\n'), text.end()); 
        result["text"].emplace_back(text);
        for (const auto& key : keys) {
            text = doc.GetMeta(key);
            text.erase(std::remove(text.begin(), text.end(), '\t'), text.end()); 
            text.erase(std::remove(text.begin(), text.end(), '\n'), text.end()); 
            result[key].emplace_back(text);
        }
        ++total;
    }

    std::ofstream ostream;
    ostream.open(output_fname);
    std::vector<std::string> keys_vec{keys.begin(), keys.end()};
    keys_vec.push_back("text");
    keys_vec.push_back("related_links");
    for (size_t i = 0; i + 1 < keys_vec.size(); ++i) {
        ostream << keys_vec[i] << "\t";
    }
    ostream << keys_vec.back() << "\n";
    for (size_t i = 0; i + 1 < total; ++i) {
        for (size_t j = 0; j + 1 < keys_vec.size(); ++j) {
            ostream << result[keys_vec[j]][i] << "\t";
        }
        ostream << result[keys_vec.back()][i] << "\n";
    }
    for (size_t j = 0; j + 1 < keys_vec.size(); ++j) {
        ostream << result[keys_vec[j]].back() << "\t";
    }
    ostream << result[keys_vec.back()].back() << "\n";
    ostream.close();
}
