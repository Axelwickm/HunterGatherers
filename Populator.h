//
// Created by axelw on 2018-12-26.
//

#ifndef HUNTERGATHERERS_POPULATOR_H
#define HUNTERGATHERERS_POPULATOR_H

#include <string>
#include <map>
#include <random>
#include "Config.h"

class World;

class Populator {
public:
    explicit Populator(World *world);

    struct Entry {
        std::string type;
        unsigned count;
        unsigned targetCount;
        float rate;
        bool enabled;
    };

    void populate(float deltaT);
    void changeCount(std::string type, int deltaCount);
    void entryEnabled(std::string type, bool enabled);

private:
    std::mt19937 randomEngine;

    std::map<std::string, Entry>& entries;
    World *world;
};


#endif //HUNTERGATHERERS_POPULATOR_H
