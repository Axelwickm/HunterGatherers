//
// Created by Axel on 2019-01-08.
//

#include <fstream>
#include <iostream>
#include <random>
#include "MarkovNames.h"
#include "Config.h"

bool MarkovNames::loaded = false;
nlohmann::json MarkovNames::chain;

std::mt19937 MarkovNames::randomEngine = std::mt19937(GeneralSettings::seed++);

void MarkovNames::loadResources() {
    if (!loaded){
        printf("Loading NamesMarkov.json\n");
        loaded = true;
        std::ifstream file("resources\\NamesMarkov.json");
        printf("Parsing NamesMarkov to json object\n");
        file >> chain;
        printf("Parsing done.\n");
    }
}

MarkovNames::MarkovNames(const bool random) : random(random) {}

std::string MarkovNames::generate(const std::vector<double> genome) {
    loadResources();
    std::string name = "";
    std::string last = "START";

    const int decisions = 10;
    std::vector<double> reducedGenome;
    reducedGenome.reserve(decisions);
    for (std::size_t i = 0; i < decisions; i++){
        reducedGenome.push_back(0);
        for (std::size_t j = 0; j < decisions; j++){
            reducedGenome.at(i) += genome.at((decisions*i+j) % (genome.size()-1)) / decisions;
        }
    }

    for (double &itr : reducedGenome) {
        // TODO: make variables be uniform instead of Bates distributed.
    }

    int lookback = 4;
    int i = 0;
    auto itr = reducedGenome.begin();

    while (name.back() != '\n'){
        std::vector<float> weights;
        weights.reserve(chain[last].size());

        if (chain.count(last) == 0){
            last = last.substr(1, last.size()-2);
            if (last.empty()){
                name = name.append("\n");
                break;
            }
            continue;
        }

        for (auto &b : chain[last]){
            weights.push_back(b[1]);
        }

        i++;
        auto dist = std::uniform_int_distribution<std::size_t>{0, weights.size()-1};
        std::size_t index = 0;
        if (random){
            index = dist(randomEngine);
        }
        else {
            index = static_cast<size_t>(floor((*itr) * weights.size()));
            itr++;
            if (itr == reducedGenome.end()){
                itr = reducedGenome.begin();
            }
        };
        auto newLast = chain[last][index][0].get<std::string>();
        name = name.append(newLast);

        last = name.substr(fmaxf((float) name.size() - lookback, 0), std::string::npos);
    }

    return name;
}

nlohmann::json chain;

