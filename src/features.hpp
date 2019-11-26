#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <thread>
#include <fstream>
#include <memory>
#include <initializer_list>
#include <unordered_map>

#include "udpipe.h"
#include "html_parse.hpp"
#include "io.hpp"
#include "common.h"

namespace udpipe = ufal::udpipe;

using pModel = std::unique_ptr<udpipe::model>;
using pReader = std::unique_ptr<udpipe::input_format>;


class FeaturesGenerator {
public:
    FeaturesGenerator() {
        for (const auto& lang : kLangs) {
            auto model_path = std::string("models/").append(lang).append(".udpipe");
            models_[lang].reset(udpipe::model::load(model_path.c_str()));
            readers_[lang].reset(models_[lang]->new_tokenizer("tokenizer"));
        }
        upostags_dict_ = ReadFeaturesNames("meta/Upostags.txt");
        feats_dict_ = ReadFeaturesNames("meta/Feats.txt");
    }

    DocFeatures GenerateFeatures(const HTMLDocument& doc, size_t sentences_count) {
        auto features = DocFeatures();
        features.bows_.emplace("title:upostags", upostags_dict_);
        features.bows_.emplace("title:feats", feats_dict_);
        features.bows_.emplace("text:upostags", upostags_dict_);
        features.bows_.emplace("text:feats", feats_dict_);

        auto lang = doc.GetMeta("lang");
        auto& reader = readers_.at(lang);
        auto& model = models_.at(lang);

        UpdateUDPipe(&features, model, reader, doc.GetText(), {"text:upostags", "text:feats"}, sentences_count);
        UpdateUDPipe(&features, model, reader, doc.GetMeta("og:title"), {"title:upostags", "title:feats"}, 100);

        return features;
    }

private:
    void UpdateFeats(DocFeatures* features, std::string_view feats, const std::string& feats_name) {
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

    void UpdateUDPipe(DocFeatures* features,
                      const pModel& model,
                      pReader& reader,
                      std::string_view text_view,
                      std::vector<std::string> features_names,
                      size_t sentences_count) {
        reader->reset_document("");
        reader->set_text(text_view.data());
        auto& upostags_bow = features->bows_.at(features_names[0]);
        for (size_t total_sentences = 0;
             (total_sentences < sentences_count) && reader->next_sentence(sentence_, error_message_);
             ++total_sentences) {
            model->tag(sentence_, udpipe::pipeline::DEFAULT, error_message_);
            //model->parse(sentence_, udpipe::pipeline::DEFAULT, error_message_);
            for (auto word = sentence_.words.begin()+1; word != sentence_.words.end(); ++word) {
                upostags_bow.Update(word->upostag);
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
    }
}


std::vector<DocFeatures> GenerateFeatures(const std::vector<HTMLDocument>& documents,
                                          size_t total_sentences) {
    std::vector<DocFeatures> features(documents.size());

    size_t step = documents.size() / kNumThreads;
    size_t begin = 0;
    size_t end = step;
    std::vector<std::thread> workers;
    for (; begin + step < documents.size(); begin+=step) {
        end = std::min(documents.size(), begin+step);
        workers.emplace_back(GenerateFeaturesTask, &features, std::cref(documents), total_sentences, begin, end);
    }
    end = std::min(documents.size(), begin+step);
    if (begin < end) {
        workers.emplace_back(GenerateFeaturesTask, &features, std::cref(documents), total_sentences, begin, end);
    }

    for (auto& worker : workers) {
        worker.join();
    }
    return features;
}
