#include "cld3.h"

#include <unordered_map>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using lang2file_t = std::unordered_map<std::string,std::vector<std::string>>;

lang2file_t SortByLanguage(const std::vector<std::string>>& filenames,
                           const fs::path& path_prefix) {
    for (const auto& filename : filenames) {
        
    }
}
