//
// Created by axelw on 2018-12-26.
//

#ifndef HUNTERGATHERERS_POPULATER_H
#define HUNTERGATHERERS_POPULATER_H

#include <string>
#include <map>
#include <random>

class World;

class Populater {
public:
    explicit Populater(World* world);

    struct Entry {
        unsigned count;
        unsigned targetCount;
        float rate;
    };

    void populate(float deltaT);
    void changeCount(std::string type, int deltaCount);
    void addEntry(std::string type, Entry entry);

private:


    std::map<std::string, Entry> frequencies;

    World* world;
    static std::mt19937 randomEngine;
};


#endif //HUNTERGATHERERS_POPULATER_H
