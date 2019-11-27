#pragma once

#include <vector>
#include <string>
#include <thread>

#include "common.h"
#include "html_parse.hpp"
#include "logreg.hpp"
#include "features.hpp"


std::vector<double> GenerateNewsDetFeatures(const DocFeatures& features) {
    auto title_upostags = features.Get("title:upostags");
    auto text_upostags = features.Get("text:upostags");
    auto title_feats = features.Get("title:feats");
    auto text_feats = features.Get("text:feats");
    AddVecMean(&title_feats, text_feats);
    title_upostags.insert(title_upostags.end(), text_upostags.begin(), text_upostags.end());
    title_upostags.insert(title_upostags.end(), title_feats.begin(), title_feats.end());
    DivVec(&title_upostags, 3);
    return title_upostags;
}


void DetectNewsTask(std::vector<DocFeatures>* documents,
                    const LogisticRegression& ru_model,
                    const LogisticRegression& en_model,
                    size_t start,
                    size_t end) {
    for (; start < end; ++start) {
        auto& doc = documents->at(start);
        if (doc.lang_ == "ru") {
            doc.is_news_score = ru_model.Infer(GenerateNewsDetFeatures(doc));
            doc.is_news_ = doc.is_news_score > 0.5;
        } else if (doc.lang_ == "en") {
            doc.is_news_score = en_model.Infer(GenerateNewsDetFeatures(doc));
            doc.is_news_ = doc.is_news_score > 0.5;
        }
    }
}


void DetectNews(std::vector<DocFeatures>* documents) {
    LogisticRegression ru_model("models/news_ru_model.logreg");
    LogisticRegression en_model("models/news_en_model.logreg");
    size_t documents_count = documents->size();
    size_t step = documents_count / kNumThreads;
    size_t start = 0;
    size_t end = step;
    std::vector<std::thread> workers;
    for (; start < documents_count; start += step) {
        end = std::min(documents_count, start+step);
        workers.emplace_back(DetectNewsTask,
                             documents,
                             std::cref(ru_model),
                             std::cref(en_model),
                             start,
                             end);
    }
    for (auto& worker : workers) {
        worker.join();
    }
}
