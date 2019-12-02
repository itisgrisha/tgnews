#include <iostream>

#include <unordered_map>
#include <string>
#include <vector>
#include <memory>

#include "common.h"


std::unordered_map<std::string, std::vector<DocFeatures>> GetThreads(const std::vector<DocFeatures>& documents) {
    std::unordered_map<std::string, std::vector<DocFeatures>> threads;
    for (const auto& document : documents) {
        threads[document.title_] = {document};
    }
    return threads;
}
