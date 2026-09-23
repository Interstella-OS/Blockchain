#ifndef CPP_RANDOM_H
#define CPP_RANDOM_H

#include <random>
#include <chrono>
#include <vector>
#include <string>
#include <algorithm>
#include <stdexcept>

namespace random {

    inline std::mt19937& engine() {
        static std::mt19937 eng(
            std::chrono::high_resolution_clock::now()
                .time_since_epoch()
                .count()
        );
        return eng;
    }

    inline void seed(unsigned int s) {
        engine().seed(s);
    }

    inline int randint(int a, int b) {
        if (a > b) std::swap(a, b);
        std::uniform_int_distribution<int> dist(a, b);
        return dist(engine());
    }

    inline int randrange(int a, int b) {
        if (a >= b) throw std::runtime_error("random::randrange() empty range");
        return randint(a, b - 1);
    }

    inline int randrange(int b) {
        return randrange(0, b);
    }

    inline double uniform(double a, double b) {
        std::uniform_real_distribution<double> dist(a, b);
        return dist(engine());
    }

    inline double random() {
        return uniform(0.0, 1.0);
    }

    template <typename T>
    const T& choice(const std::vector<T>& v) {
        if (v.empty()) throw std::runtime_error("random::choice() empty vector");
        std::uniform_int_distribution<size_t> dist(0, v.size() - 1);
        return v[dist(engine())];
    }

    template <typename T, size_t N>
    const T& choice(const T (&arr)[N]) {
        std::uniform_int_distribution<size_t> dist(0, N - 1);
        return arr[dist(engine())];
    }

    inline char choice(const std::string& s) {
        if (s.empty()) throw std::runtime_error("random::choice() empty string");
        std::uniform_int_distribution<size_t> dist(0, s.size() - 1);
        return s[dist(engine())];
    }

    template <typename T>
    void shuffle(std::vector<T>& v) {
        std::shuffle(v.begin(), v.end(), engine());
    }

    inline void shuffle(std::string& s) {
        std::shuffle(s.begin(), s.end(), engine());
    }

    template <typename T>
    std::vector<T> sample(const std::vector<T>& v, size_t k) {
        if (k > v.size()) throw std::runtime_error("random::sample() k > size");
        
        std::vector<T> result;
        result.reserve(k);
        
        if (k * 5 > v.size()) {
            std::vector<T> temp = v;
            std::shuffle(temp.begin(), temp.end(), engine());
            result.assign(temp.begin(), temp.begin() + k);
        } else {
            std::vector<size_t> indices;
            indices.reserve(k);
            while (indices.size() < k) {
                std::uniform_int_distribution<size_t> dist(0, v.size() - 1);
                size_t idx = dist(engine());
                if (std::find(indices.begin(), indices.end(), idx) == indices.end()) {
                    indices.push_back(idx);
                    result.push_back(v[idx]);
                }
            }
        }
        return result;
    }

    inline bool chance(double p) {
        return random() < p;
    }

}

#endif