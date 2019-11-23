#include <unordered_map>
#include <string>
#include <vector>

#include "cld3/nnet_language_identifier.h"
#include "html_parse.hpp"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

using lang2file_t = std::unordered_map<std::string,std::vector<std::string>>;

json SortByLanguage(const std::vector<std::string>& filenames, const std::string& prefix) {
    size_t prefix_len = prefix.size() + (prefix.back() != '/');
    json result = {};
    chrome_lang_id::NNetLanguageIdentifier langrec(0, 1000);
    for (const auto& filename : filenames) {
        HTMLDocument doc(filename);
        auto doc_lang = langrec.FindLanguage(doc.GetText());
        if (doc_lang.is_reliable && (doc_lang.language == "ru" || doc_lang.language == "en")) {
            result[doc_lang.language].emplace_back(filename.substr(prefix_len));
        }
    }
    return result;
}
