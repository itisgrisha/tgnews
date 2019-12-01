#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <thread>
#include <fstream>
#include <memory>
#include <initializer_list>
#include <regex>
#include <unordered_map>
#include <filesystem>

#include <boost/algorithm/string.hpp>

#include "udpipe.h"
#include "html_parse.hpp"
#include "io.hpp"
#include "common.h"


namespace udpipe = ufal::udpipe;
namespace fs = std::filesystem;

using pModel = std::unique_ptr<udpipe::model>;
using pReader = std::unique_ptr<udpipe::input_format>;


class FeaturesGenerator {
public:
    FeaturesGenerator() {
        auto prefix = fs::path("models");
        for (const auto& lang : kLangs) {
            auto model_name = std::string(lang).append(".udpipe");
            auto model_path = (prefix / lang / model_name).string();
            models_[lang].reset(udpipe::model::load(model_path.c_str()));
            readers_[lang].reset(models_[lang]->new_tokenizer("tokenizer")->new_generic_tokenizer_input_format());
        }
        upostags_dict_ = ReadFeaturesNames("meta/Upostags.txt");
        feats_dict_ = ReadFeaturesNames("meta/Feats.txt");
        lemmas_dict_ = ReadLemmas("meta/Lemmas.txt");
    }

    DocFeatures GenerateFeatures(const HTMLDocument& doc, int sentences_count) {
        auto features = DocFeatures();
        features.bows_.emplace("title:upostags", upostags_dict_);
        features.bows_.emplace("title:feats", feats_dict_);
        features.bows_.emplace("text:upostags", upostags_dict_);
        features.bows_.emplace("text:feats", feats_dict_);

        auto lang = doc.GetMeta("lang");
        auto& reader = readers_.at(lang);
        auto& model = models_.at(lang);

        if (sentences_count < 0) {
            sentences_count = 1'000'000;
        }

        UpdateUDPipe(&features, model, reader, doc.GetText(),
            {"text:upostags", "text:feats", "text"}, sentences_count);
        UpdateUDPipe(&features, model, reader, doc.GetMeta("og:title"),
            {"title:upostags", "title:feats", "title"}, sentences_count);

        return features;
    }

private:
    std::string CleanText(const std::string& input_text) {
        std::regex https("https*:\\S*");
        std::regex word("[^[\\p{L}]+\\.\\!\\?\\(\\)\\-:;,]+");
        return std::regex_replace(std::regex_replace(input_text, https, " "), word, " ");
    }

    void UpdateFeats(DocFeatures* features,
                     std::string_view feats,
                     const std::string& feats_name) {
        auto begin = feats.begin();
        auto end = feats.begin();
        auto& feats_bow = features->bows_.at(feats_name);
        while (end != feats.end()) {
            if (*end == '|') {
                feats_bow.Update({&*begin, end-begin});
                begin = end + 1;
            }
            ++end;
        }
        if (begin != end) {
            feats_bow.Update({&*begin, end-begin});
        }
    }

    void UpdateLemma(DocFeatures* features,
                     std::string lemma,
                     std::shared_ptr<BOWDict> lemmas_dict,
                     const std::string& class_id) {
        boost::erase_all(lemma, ":");
        auto it = lemmas_dict->find(lemma);
        if (it != lemmas_dict->end()) {
            ++features->lemmas_[class_id][it->second];
        }
    }

    void UpdateUDPipe(DocFeatures* features,
                      const pModel& model,
                      pReader& reader,
                      const std::string& input_text,
                      std::vector<std::string> features_names,
                      int sentences_count) {
        //std::cout << "BEFORE\n" << input_text << std::endl;
        auto text = CleanText(input_text);
        //std::cout << "AFTER\n" << text << std::endl;

        //for (int i = 0; i < 80; ++i){
        //    std::cout << "-";
        //}
        //std::cout << std::endl;
        reader->reset_document("");
        reader->set_text(text.c_str());
        auto& upostags_bow = features->bows_.at(features_names[0]);
        std::unordered_set<std::string> lemma_upostags = {"ADJ", "INTJ", "NOUN", "PROPN", "VERB"};
        auto lemmas_dict = lemmas_dict_.at(features_names[2]);
        for (int total_sentences = 0;
             (total_sentences < sentences_count) && reader->next_sentence(sentence_, error_message_);
             ++total_sentences) {
            model->tag(sentence_, udpipe::pipeline::DEFAULT, error_message_);
            for (auto word = sentence_.words.begin()+1; word != sentence_.words.end(); ++word) {
                upostags_bow.Update(word->upostag);
                if (lemma_upostags.find(word->upostag) != lemma_upostags.end()) {
                    UpdateLemma(features, word->lemma, lemmas_dict, features_names[2]);
                }
                UpdateFeats(features, word->feats, features_names[1]);
            }
        }
    }

    std::unordered_map<std::string, pModel> models_;
    std::unordered_map<std::string, pReader> readers_;
    udpipe::sentence sentence_;
    std::string error_message_;

    std::shared_ptr<BOWDict> upostags_dict_;
    std::shared_ptr<BOWDict> feats_dict_;
    std::unordered_map<std::string, std::shared_ptr<BOWDict>> lemmas_dict_;
    std::unordered_set<std::string> categories_;

    static constexpr std::initializer_list kLangs = {"ru", "en"};
};


void GenerateFeaturesTask(std::vector<DocFeatures>* features,
                          const std::vector<HTMLDocument>& docs,
                          size_t total_sentences,
                          size_t start,
                          size_t end) {
    FeaturesGenerator features_generator;
    for (size_t i = start; i < end; ++i) {
        auto& doc = docs.at(i);
        auto& feature = features->at(i);
        auto lang = doc.GetMeta("lang");
        if (lang == "ru" or lang == "en") {
            feature = features_generator.GenerateFeatures(docs.at(i), total_sentences);
        }
        feature.doc_name_ = doc.GetMeta("path");
        feature.lang_ = lang;
        feature.url_ = doc.GetMeta("og:url");
    }
}


std::vector<DocFeatures> GenerateFeatures(const std::vector<HTMLDocument>& documents,
                                          size_t total_sentences) {
    std::vector<DocFeatures> features(documents.size());

    size_t step = std::max<size_t>(1u, documents.size() / kNumThreads);
    size_t begin = 0;
    size_t end = step;
    std::vector<std::thread> workers;
    for (; begin < documents.size(); begin+=step) {
        end = std::min(documents.size(), begin+step);
        workers.emplace_back(GenerateFeaturesTask, &features, std::cref(documents), total_sentences, begin, end);
    }
    for (auto& worker : workers) {
        worker.join();
    }
    return features;
}
