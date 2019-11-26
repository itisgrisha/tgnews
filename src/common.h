#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <memory>


constexpr size_t kNumThreads = 6;

using BOWDict = std::unordered_map<std::string, size_t>;

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
    std::vector<double> bag_;
    std::shared_ptr<BOWDict> dict_;
};


struct DocFeatures {
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

    std::unordered_map<std::string, BOW> bows_;
    std::string doc_name_;
    std::string lang_;
    bool is_news_ = false;
};
