#pragma once

#include <vector>
#include <fstream>
#include <iostream>
#include <cmath>
#include <iomanip>


class LogisticRegression {
public:
    LogisticRegression(const std::string& weights_path) {
        std::ifstream input(weights_path);
        input >> features_count_;
        weights_.resize(features_count_);
        mean_.resize(features_count_);
        std_.resize(features_count_);
        input >> bias_;
        for (size_t pos = 0; input >> weights_[pos] >> mean_[pos] >> std_[pos]; ++pos) {
            if (std_[pos] > 0) {
                weights_[pos] = weights_[pos] / std_[pos];
            }
        }
    }

    double Infer(std::vector<double> features) const {
        double result = 0;
        for (size_t i = 0; i < features_count_; ++i) {
            result += (features[i] - mean_[i]) * weights_[i];
        }
        return 1. / (1. + std::exp(-(result + bias_)));
    }


private:
    std::vector<double> weights_;
    double bias_;
    size_t features_count_;
    std::vector<double> mean_;
    std::vector<double> std_;
};


