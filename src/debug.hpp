#include <fstream>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <algorithm>

#include "html_parse.hpp"

std::vector<HTMLDocument> LoadDocs(const std::vector<std::string>& files) {
    std::vector<HTMLDocument> documents;
    for (const auto& file : files) {
        documents.emplace_back(file);
        documents.back().SetMeta("path", file);
    }
    return documents;
}

void MakeTsv(const std::vector<HTMLDocument>& documents, const std::string& output_fname) {
    std::unordered_set<std::string> keys;
    for (const auto& doc : documents) {
        for (const auto& key : doc.GetMetaKeys()) {
            keys.emplace(key);
        }
    }
    std::unordered_map<std::string, std::vector<std::string>> result;
    size_t total = 0;
    for (const auto& doc : documents) {
        //if (doc.GetMeta("path") == "/eee/tgnews/data/20191121/00/2653864115930851288.html") {
        //    std::cout << "KEK" <<std::endl;
        //}
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
            //if (result[keys_vec[j]][i] == "/eee/tgnews/data/20191121/00/2653864115930851288.html") {
            //    std::cout << "kaka" << std::endl;
            //}
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
