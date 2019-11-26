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

#include "typenames.hpp"
#include "udpipe.h"
#include "html_parse.hpp"
#include "io.hpp"


class BOW {
public:
    BOW(std::shared_ptr<BOWDict> dict)
        : bag_(std::vector<double>(dict->size(), 0)),
          dict_(dict) {
    }

    void Update(const std::string& word) {
        auto it = dict_->find(word);
        if (it != dict_->end()) {
            ++bag_.at(it->second);
        }
    }

    std::vector<double> const & Get() const {
        return bag_;
    }

    void Normalize() {
        double denumerator = 0;
        for (const auto& v : bag_) {
            denumerator += v;
        }
        if (denumerator > 0) {
            for (auto& v : bag_) {
                v /= denumerator;
            }
        }
    }

    void Clear() {
        std::fill(bag_.begin(), bag_.end(), 0);
    }

private:
    std::shared_ptr<BOWDict> dict_;
    std::vector<double> bag_;
};


struct DocFeatures {
    std::vector<double> Concat(const std::vector<std::string>& features_names) {
        std::vector<double> result;
        for (const auto& name : features_names) {
            bows_.at(name).Normalize();
            for (const auto& v : bows_.at(name).Get()) {
                result.emplace_back(v);
            }
        }
        return result;
    }

    std::unordered_map<std::string, BOW> bows_;
    std::string doc_name_;
};


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

    DocFeatures GenerateFeatures(const HTMLDocument& doc) {
        auto features = DocFeatures();
        features.bows_.emplace("title:upostags", upostags_dict_);
        features.bows_.emplace("title:feats", feats_dict_);
        features.bows_.emplace("text:upostags", upostags_dict_);
        features.bows_.emplace("text:feats", feats_dict_);

        auto lang = doc.GetMeta("lang");
        auto& reader = readers_.at(lang);
        auto& model = models_.at(lang);

        UpdateUDPipe(&features, model, reader, doc.GetText(), {"text:upostags", "text:feats"});
        UpdateUDPipe(&features, model, reader, doc.GetMeta("og:title"), {"title:upostags", "title:feats"});

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
                      std::vector<std::string> features_names) {
        size_t total_tokens = 0;
        reader->reset_document("");
        reader->set_text(text_view.data());
        auto& upostags_bow = features->bows_.at(features_names[0]);
        while (reader->next_sentence(sentence_, error_message_)) {
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


void GenerateFeaturesTask(std::vector<DocFeatures>* features, const std::vector<HTMLDocument>& docs, size_t start, size_t end) {
    FeaturesGenerator features_generator;
   for (size_t i = start; i < end; ++i) {
       auto& doc = docs.at(i);
       if (doc.GetMeta("lang") == "ru" or doc.GetMeta("lang") == "en") {
           features->at(i) = features_generator.GenerateFeatures(docs.at(i));
       }
       features->at(i).doc_name_ = doc.GetMeta("path");
   }
}


std::vector<DocFeatures> GenerateFeatures(const std::vector<HTMLDocument>& documents) {
    std::vector<DocFeatures> features(documents.size());

    size_t step = documents.size() / 4;
    size_t begin = 0;
    size_t end = step;
    std::vector<std::thread> workers;
    for (; begin + step < documents.size(); begin+=step) {
        end = std::min(documents.size(), begin+step);
        workers.emplace_back(GenerateFeaturesTask, &features, std::cref(documents), begin, end);
    }
    end = std::min(documents.size(), begin+step);
    if (begin < end) {
        workers.emplace_back(GenerateFeaturesTask, &features, std::cref(documents), begin, end);
    }

    for (auto& worker : workers) {
        worker.join();
    }
    return features;

    //FeaturesGenerator features_generator;
    //for (const auto& doc : docs) {
    //    if (doc.GetMeta("lang") != "ru" and doc.GetMeta("lang") != "en") {
    //        continue;
    //    }
    //    auto features = features_generator.GenerateFeatures(doc); 
    //    //for (const auto& f : features.Concat({"title:upostags", "text:upostags", "title:feats", "text:feats"})) {
    //    //    std::cout << f << " ";
    //    //}
    //    //std::cout << "\n";
    //}
}
