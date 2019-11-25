#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <memory>
#include <initializer_list>

#include "udpipe.h"
#include "html_parse.hpp"
#include "utils.hpp"

namespace udpipe = ufal::udpipe;

typename pModel = std::unique_ptr<udpipe::model>;
typename pReader = std::unique_ptr<udpipe::input_format>;
typename pBOWDict = std::shared_ptr<std::string, size_t>;


class BOW {
public:
    BOW() = delete;

    BOW(pBOWDict dict)
        : bag_({dict.size(), 0}),
          dict_(dict);

    void Update(const std::string& word) {
        ++bag_[dict_->at(word)];
    }

    std::vector<double> const & Get() const {
        return bag_;
    }

    void Normalize() {
        double denumerator = 0;
        for (const auto& v : bag_) {
            denumerator += v;
        }
        for (auto& v : bag_) {
            v /= denumerator;
        }
    }

    void Clear() {
        std::fill(bag_.begin(), bag_.end(), 0);
    }

private:
    pBOWDict dict_;
    std::vector<double> bag_;
}


struct DocFeatures {
    std::vector<double> Concat(const std::vector<std::string>& features_names) {
        std::vector<double> result;
        for (const auto& name : features_names) {
            bows_.at(name).Normalize();
            for (const auto& v : bows_.at(name)) {
                result.emplace_back(v);
            }
        }
        return result;
    }

    std::unordered_map<std::string, BOW> bows_;
};


class FeaturesGenerator {
public:
    FeaturesGenerator() {
        for (const auto& lang : kLangs) {
            models_[lang].reset(udpipe::model::load(std::string("models/").append(lang).append(".udpipe")));
            readers_[lang].reset(models[lang]->new_tokenizer("tokenizer"));
        }
        upostag_dict_ = std::make_shared(ReadFeaturesNames("meta/Upostags.txt"));
        feat_dict_ = std::make_shared(ReadFeaturesNames("meta/Feats.txt"));
    }

    DocFeatures GenerateFeatures(const HTMLDocument& doc) {
        auto features = DocFeatures();
        features.bows_.emplace("title:upostags", upostag_dict_);
        features.bows_.emplace("title:feats", feats_dict_);
        features.bows_.emplace("text:upostags", upostag_dict_);
        features.bows_.emplace("text:feats", feats_dict_);

        auto lang = doc.GetMeta("lang");
        auto& reader = readers_.at(lang);
        auto& model = models_.at(lang);

        UpdateUDPipe(&features, model, reader, doc.GetText(), {"text:upostags", "text:feats"});
        UpdateUDPipe(&features, model, reader, doc.GetMeta("og:title"), {"title:upostags", "title:feats"});

        features.Normalize();
        return features;
    }

private:
    void UpdateFeats(DocFeatures* features, std::string_view feats, const std::string& feats_name) {
        auto begin = feats.begin();
        auto end = feats.begin();
        while (end != feats.end()) {
            if (*end == '|') {
                features->bows_[feats_name].Update(feats[feats_map_.at({&*begin, end-begin})]);
                begin = end + 1;
            }
            ++end;
        }
        if (begin != end) {
            features->bows_[feats_name].Update(feats[feats_map_.at({&*begin, end-begin})]);
        }
    }

    void UpdateUDPipe(DocFeatures* features,
                      const pModel& model,
                      pReader& reader,
                      std::string_view text_view,
                      std::vector<std::string> features_names) {
        size_t total_tokens = 0;
        reader->reset_document("");
        reader->set_text(text_view.data());
        while (reader->next_sentence(sentence_, error_message_)) {
            model->tag(sentence_, udpipe::pipeline::DEFAULT, error_message_);
            model->parse(sentence_, udpipe::pipeline::DEFAULT, error_message_);
            for (auto word = sentence_.words.begin()+1, word != sentence_.words.end(); ++word) {
                features->bows_.at(features_names[0]).Update(word->upostag);
                UpdateFeats(features, word->feats, features_names[1]);
            }
        }
    }

    std::unordered_map<std::string, pModel> models_;
    std::unordered_map<std::string, pReader> readers_;
    udpipe::sentence sentence_;
    std::string error_message_;

    pBOWDict upostag_dict_;
    pBOWDict feat_dict_;

    static constexpr std::vector<std::string> kLangs = {"ru", "en"};
};



void GenerateFeatures(const std::vector<HTMLDocument>& docs) {
    FeaturesGenerator features_generator;
    for (const auto& doc : docs) {
        if (doc.GetMeta("lang") != "ru" and doc.GetMeta("lang") != "en") {
            continue;
        }
        auto features = features_generator.GenerateFeatures(doc); 
    }
}
