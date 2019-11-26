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


