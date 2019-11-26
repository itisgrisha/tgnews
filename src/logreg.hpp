#pragma once

#include <vector>
#include <fstream>
#include <iostream>
#include <cmath>


class LogisticRegression {
public:
    LogisticRegression(const std::string& weights_path) {
        std::ifstream input(weights_path);
        input >> features_count_;
        weights_.resize(features_count);
        mean_.resize(features_count);
        std_.resize(features_count);
        input >> bias_;
        double std_;
        for (size_t pos = 0; input >> weights_[pos] >> mean_[pos] >> std_; ++pos) {
            if (std_[pos] > 0) {
                weights_[pos] = weights_[pos] / std_[pos];
            }
        }
    }

    double Infer(const std::vector<double>& features) {
        double result = 0;
        for (size_t i = 0; i < features_count_; ++i) {
            result += (features[i] - mean_[i]) * weights_[pos];
        }
        return std::exp(-(result + bias_));
    }

private:
    std::vector<double> weights_;
    double bias_;
    size_t features_count_;
    std::vector<double> mean_;
    std::vector<double> std_;
}
