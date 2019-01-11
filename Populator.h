//
// Created by axelw on 2018-12-26.
//

#ifndef HUNTERGATHERERS_POPULATOR_H
#define HUNTERGATHERERS_POPULATOR_H

#include <string>
#include <map>
#include <random>

class World;

class Populator {
public:
    explicit Populator(World *world);

    struct Entry {
        std::string type;
        unsigned count;
        unsigned targetCount;
        float rate;
    };

    void populate(float deltaT);
    void changeCount(std::string type, int deltaCount);
    void addEntry(Entry entry);
    void addEntries(std::vector<Populator::Entry> entries);

private:


    std::map<std::string, Entry> frequencies;

    World *world;
    std::mt19937 randomEngine;
};


#endif //HUNTERGATHERERS_POPULATOR_H
