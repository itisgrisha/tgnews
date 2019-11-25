#pragma once

#include <glob.h>
#include <string.h>
#include <string>
#include <iostream>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

bool IsHTML(const std::string& file_name) {
    return file_name.compare(file_name.size()-5, 5, ".html") == 0;
}

std::vector<std::string> GlobHTML(const std::string& input_folder) {
    std::vector<std::string> html_files;
    for (const auto& directory_entry : fs::recursive_directory_iterator(input_folder)) {
        if (fs::is_regular_file(directory_entry)) {
            auto path_name = directory_entry.path().string();
            if (IsHTML(path_name)) {
                html_files.emplace_back(path_name);
            }
        }
    }
    return html_files;
}


std::unordered_map<std::string, size_t> ReadFeaturesNames(const std::string& path) {
    std::unordered_map<std::string, size_t> features_names;
    std::ifstream input(path);
    std::string feature;
    for (size_t feature_pos = 0; input >> feature; ++feature_pos) {
        features_names[feature] = feature_pos;
    }
    return features_names;
}
