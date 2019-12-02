
#include <iostream>

#include <unordered_map>
#include <string>
#include <vector>
#include <memory>

#include "common.h"


std::unordered_map<std::string, std::unordered_map<std::string, std::vector<DocFeatures>>> GetThreads(std::shared_ptr<std::unordered_map<std::string, std::vector<DocFeatures>>> cats2docs) {
    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<DocFeatures>>> cats2threads;
    for (const auto& cat2docs : *cats2docs) {

    }
    return cats2threads;
}
