#pragma once

#include <vector>
#include <string>
#include <regex>
#include <thread>

#include "common.h"
#include "html_parse.hpp"
#include "logreg.hpp"
#include "features.hpp"


std::vector<double> GenerateNewsDetFeatures(const DocFeatures& features) {
    auto title_upostags = features.Get("title:upostags");
    //auto text_upostags = features.Get("text:upostags");
    //auto text_feats = features.Get("text:feats");
    //title_upostags.insert(title_upostags.end(), text_upostags.begin(), text_upostags.end());
    //title_upostags.insert(title_upostags.end(), text_feats.begin(), text_feats.end());
    //DivVec(&title_upostags, 3);
    return title_upostags;
}


void DetectNewsTask(std::vector<DocFeatures>* documents,
                    const LogisticRegression& ru_model,
                    const LogisticRegression& en_model,
                    size_t start,
                    size_t end) {
    std::regex news("(news|novosti)");
    std::regex https("https*://[^/]*");

    for (; start < end; ++start) {
        auto& doc = documents->at(start);
        auto url = std::regex_replace(doc.url_, https, "");
        if (std::regex_search(url, news)) {
            doc.is_news_score = 1;
            doc.is_news_ = true;
        } else if (doc.lang_ == "ru") {
            doc.is_news_score = ru_model.Infer(GenerateNewsDetFeatures(doc));
            doc.is_news_ = doc.is_news_score > 0.5;
        } else if (doc.lang_ == "en") {
            doc.is_news_score = en_model.Infer(GenerateNewsDetFeatures(doc));
            doc.is_news_ = doc.is_news_score > 0.5;
        }
        //std::cout << url << " is news: " << doc.is_news_score << std::endl;
    }
}


std::vector<DocFeatures> DetectNews(std::vector<DocFeatures>* documents) {
    //std::cout << documents->size() << std::endl;
    LogisticRegression ru_model("models/ru/news_ru_model.logreg");
    LogisticRegression en_model("models/en/news_en_model.logreg");
    size_t documents_count = documents->size();
    size_t step = std::max<size_t>(1u, documents_count / kNumThreads);
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
    std::vector<DocFeatures> news;
    for (auto& doc : *documents) {
        if (doc.is_news_) {
            news.emplace_back(std::move(doc));
        }
    }
    return news;
}
