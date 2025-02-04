#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <memory>


constexpr size_t kNumThreads = 8;

using BOWDict = std::unordered_map<std::string, size_t>;


void AddVecMean(std::vector<double>* first, const std::vector<double>& second) {
    double denumerator = 2; 
    for (size_t i = 0; i < second.size(); ++i) {
        first->at(i) = (first->at(i) + second.at(i)) / denumerator;
    }
}

void DivVec(std::vector<double>* vec, double denumerator) {
    for (auto& v : *vec) {
        v /= denumerator;
    }
}


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

    std::vector<double> Get() const {
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
    std::vector<double> bag_;
    std::shared_ptr<BOWDict> dict_;
};


struct DocFeatures {
    DocFeatures() {
        lemmas_["text"] = {};
        lemmas_["title"] = {};
    }

    std::vector<double> Concat(const std::vector<std::string>& features_names) {
        std::vector<double> result;
        for (const auto& name : features_names) {
            bows_.at(name).Normalize();
            for (const auto& v : bows_.at(name).Get()) {
                result.emplace_back(v / features_names.size());
            }
        }
        return result;
    }

    std::vector<double> Get(const std::string& feature_name) const {
        auto bow = bows_.at(feature_name);
        bow.Normalize();
        return bow.Get();
    }

    std::unordered_map<std::string, BOW> bows_;
    std::unordered_map<std::string, std::unordered_map<size_t, float>> lemmas_;
    std::string title_;
    std::string doc_name_;
    std::string lang_;
    std::string url_;
    bool is_news_ = false;
    double is_news_score = 0;
};
