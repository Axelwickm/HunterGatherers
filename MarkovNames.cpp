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
        file.close();
        printf("Parsing done.\n");
    }
}

MarkovNames::MarkovNames(const bool random) : random(random) {}

std::string MarkovNames::generate(const std::vector<double> genome) {
    loadResources();
    std::string name = "";
    std::string last = "START";

    const int decisions = 8;
    const float k = 10;
    const double commonBias = 1.15;

    std::vector<double> reducedGenome;
    reducedGenome.reserve(decisions);
    for (std::size_t i = 0; i < decisions; i++){
        reducedGenome.push_back(0);
        for (std::size_t j = 0; j < decisions; j++){
            reducedGenome.at(i) += genome.at((decisions*i+j) % (genome.size()-1)) / decisions;
        }
    }


    for (double &itr : reducedGenome) {
        // Logistic function to make values more uniform
        itr = pow(itr, commonBias);
        itr = 1.f / (1.f + expf(float(-k * (itr - 0.5f))));
    }

    int lookback = 4;
    int i = 0;
    auto itr = reducedGenome.begin();

    while (true){
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
            if (index == chain[last].size()){
                index--;
            }
            itr++;
            if (itr == reducedGenome.end()){
                itr = reducedGenome.begin();
            }
        };

        auto newLast = chain[last][index][0].get<std::string>();
        if (newLast == "\n"){
            break;
        }
        name = name.append(newLast);

        last = name.substr(fmaxf((float) name.size() - lookback, 0), std::string::npos);
    }

    return name;
}

nlohmann::json chain;

