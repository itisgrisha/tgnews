#include <unordered_map>
#include <iostream>
#include <string>
#include <vector>

#include "cld3/nnet_language_identifier.h"
#include "html_parse.hpp"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

using lang2file_t = std::unordered_map<std::string,std::vector<std::string>>;

void RecognizeLanguage(std::vector<HTMLDocument>* documents) {
    chrome_lang_id::NNetLanguageIdentifier langrec(0, 1000);
    for (auto& html_doc : *documents) {
        auto langrec_result = langrec.FindLanguage(html_doc.GetText());
        html_doc.SetMeta("lang", langrec_result.language);
        html_doc.SetMeta("lang_score", std::to_string(langrec_result.probability));
    }
}

