//
// Created by Axel on 2019-01-08.
//

#ifndef HUNTERGATHERERS_MARKOVNAMES_H
#define HUNTERGATHERERS_MARKOVNAMES_H


#include "json/json.hpp"

class MarkovNames {
public:
    MarkovNames(const bool random, unsigned long seed);

    static void loadResources();
    std::string generate(const std::vector<double> genome);

private:
    const bool random;

    static bool loaded;
    static nlohmann::json chain;

    std::mt19937 randomEngine;
};


#endif //HUNTERGATHERERS_MARKOVNAMES_H
