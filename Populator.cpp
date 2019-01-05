//
// Created by axelw on 2018-12-26.
//

#include <ctime>
#include "World.h"

std::mt19937 Populator::randomEngine = std::mt19937(std::random_device()());

Populator::Populator(World* world) : world(world) {}

void Populator::populate(float deltaT) {
    for (auto& entry : frequencies){
        if (entry.second.count < entry.second.targetCount){
            auto d = std::poisson_distribution(entry.second.rate*deltaT);
            int newCount = d(randomEngine);
            for (unsigned i = 0; i < newCount; i++){
                world->spawn(entry.first);
                if (entry.second.targetCount <= entry.second.count ){
                    break;
                }
            }
        }
    }
}

void Populator::addEntry(Entry entry) {
    frequencies.insert(std::make_pair(entry.type, entry));
}

void Populator::addEntries(std::vector<Populator::Entry> entries) {
    for (auto&e : entries){
        addEntry(e);
    }
}

void Populator::changeCount(std::string type, int deltaCount) {
    auto itr = frequencies.find(type);
    if (itr != frequencies.end()){
        frequencies.at(type).count += deltaCount;
    }
}
