
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <initializer_list>
#include <memory>
#include <filesystem>

#include "common.h"
#include "artm/cpp_interface.h"
#include "artm/messages.pb.h"
#include <nlohmann/json.hpp>


using json = nlohmann::json;
namespace fs = std::filesystem;


class ARTM {
public:
    explicit ARTM(const std::string& lang) : lang_(lang) {
        auto prefix = fs::path("models") / lang;
        auto model_path = prefix / "cl_news_model.save_tm";
        auto dict_path = prefix / "dictionary.dict";

        artm::MasterModelConfig master_model_config;
        master_model_config.set_pwt_name(std::string("pwt_").append(lang));
        model_.reset(new artm::MasterModel(master_model_config));

        artm::ImportModelArgs import_model_args;
        import_model_args.set_file_name(model_path.string());
        import_model_args.set_model_name(master_model_config.pwt_name());
        model_->ImportModel(import_model_args);

        artm::ImportDictionaryArgs import_dictionary_args;
        import_dictionary_args.set_file_name(dict_path.string());
        import_dictionary_args.set_dictionary_name(lang);
        model_->ImportDictionary(import_dictionary_args);

        artm::GetDictionaryArgs get_dictionary_args;
        get_dictionary_args.set_dictionary_name(lang);
        dict_ = model_->GetDictionary(get_dictionary_args);
    }


    void AddCategories(const std::vector<DocFeatures>& documents,
                       std::shared_ptr<std::unordered_map<std::string,
                                                          std::vector<DocFeatures>>> cat2doc) {
        artm::TransformMasterModelArgs transform_master_model_args;

        auto batch = transform_master_model_args.add_batch();
        batch->set_id(lang_);
        for (int token_idx = 0; token_idx < dict_.token_size(); ++token_idx) {
            batch->add_token(dict_.token(token_idx));
            batch->add_class_id(dict_.class_id(token_idx));
        }
        for (const auto& doc : documents) {
            auto item = batch->add_item();
            item->set_title(doc.doc_name_);
            for (const auto& class_id : {"text", "title"}) {
                for (const auto& lemma2count : doc.lemmas_.at(class_id)) {
                    item->add_token_id(lemma2count.first);
                    item->add_token_weight(lemma2count.second);
                }
            }
        }

        auto theta = model_->Transform(transform_master_model_args);

        auto num_topics = theta.num_topics();
        auto num_docs = theta.item_title_size();
        for (size_t doc_idx = 0; doc_idx < num_docs; ++doc_idx) {
            for (size_t cat_idx = 0; cat_idx + 1 < num_topics; ++cat_idx) {
                if (theta.item_weights(doc_idx).value(cat_idx) > 0.2) {
                    cat2doc->at(theta.topic_name(cat_idx)).emplace_back(documents.at(doc_idx));
                }
            }
        }
    }

private:
    std::shared_ptr<artm::MasterModel> model_;
    artm::DictionaryData dict_;
    std::string lang_;
};


std::shared_ptr<std::unordered_map<std::string, std::vector<DocFeatures>>> GetCategories(const std::vector<DocFeatures>& documents) {
    std::vector<std::string> categories = {
        "Society",
        "Economy",
        "Technology",
        "Sports",
        "Entertainment",
        "Science",
        "Other"
    };
    std::vector<std::string> langs = {"ru", "en"};
    std::unordered_map<std::string, ARTM> lang2model = {
        {"ru", ARTM("ru")},
        {"en", ARTM("en")}
    };
    std::unordered_map<std::string, std::vector<DocFeatures>> lang2docs;
    for (const auto& doc : documents) {
        lang2docs[doc.lang_].emplace_back(doc);
    }
    std::shared_ptr<std::unordered_map<std::string, std::vector<DocFeatures>>> cat2doc;
    cat2doc.reset(new std::unordered_map<std::string, std::vector<DocFeatures>>());
    for (const auto& cat : categories) {
        cat2doc->insert({cat, {}});
    }
    for (const auto& lang : langs) {
        lang2model.at(lang).AddCategories(lang2docs.at(lang), cat2doc);
    }
    return cat2doc;
}

