#pragma once

#include <vector>
#include <string>
#include <thread>

#include "common.h"
#include "html_parse.hpp"
#include "logreg.hpp"
#include "features.hpp"


void DetectNewsTask(std::vector<DocFeatures>* documents,
                    const LogisticRegression& ru_model,
                    const LogisticRegression& en_model,
                    size_t start,
                    size_t end) {
    std::vector<std::string> features_names = {
        "title:upostags",
        "text:upostags",
        "title:feats"
    };
    for (; start < end; ++start) {
        auto& doc = documents->at(start);
        if (doc.lang_ == "ru") {
            doc.is_news_ = ru_model.Infer(doc.Concat(features_names)) > 0.5;
        } else if (doc.lang_ == "en") {
            doc.is_news_ = en_model.Infer(doc.Concat(features_names)) > 0.5;
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
