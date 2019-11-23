#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <algorithm>

#include "html_parse.hpp"


void MakeTsv(const std::vector<std::string>& files, const std::string& output_fname) {
    std::unordered_map<std::string, std::vector<std::string>> result;
    size_t total = 0;
    for (const auto& file : files) {
        HTMLDocument doc(file);
        result["path"].emplace_back(file);
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
        for (const auto& key : doc.GetMetaKeys()) {
            auto it = result.find(key);
            if (it == result.end()) {
                result.emplace(std::make_pair(key, std::vector<std::string>(total, "")));
            }
            text = doc.GetMeta(key);
            text.erase(std::remove(text.begin(), text.end(), '\t'), text.end()); 
            text.erase(std::remove(text.begin(), text.end(), '\n'), text.end()); 
            result[key].emplace_back(text);
        }
        ++total;
    }

    std::ofstream ostream;
    ostream.open(output_fname);
    std::vector<std::string> keys;
    for (const auto& key : result) {
        keys.emplace_back(key.first);
    }
    for (int i = 0; i + 1 < keys.size(); ++i) {
        ostream << keys[i] << "\t";
    }
    ostream << keys.back() << "\n";
    for (int i = 0; i + 1 < total; ++i) {
        for (int j = 0; j + 1 < keys.size(); ++j) {
            ostream << result[keys[j]][i] << "\t";
        }
        ostream << result[keys.back()][i] << "\n";
    }
    for (int j = 0; j + 1 < keys.size(); ++j) {
        ostream << result[keys[j]].back() << "\t";
    }
    ostream << result[keys.back()].back() << "\n";
    ostream.close();
}
