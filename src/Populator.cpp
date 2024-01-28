//
// Created by axelw on 2018-12-26.
//

#include <ctime>
#include "World.h"
#include "Populator.h"


Populator::Populator(World *world) : world(world), entries(world->getConfig().world.populatorEntries) {
    randomEngine = std::mt19937(world->getConfig().seed++);
}

void Populator::populate(float deltaT) {
    for (auto &entry : entries) {
        if (entry.second.count < entry.second.targetCount && entry.second.enabled){
            auto d = std::poisson_distribution(entry.second.rate*deltaT);
            int newCount = d(randomEngine);
            for (unsigned i = 0; i < newCount; i++){
                if (!world->spawn(entry.first)){
                    fprintf(stderr, "Could not spawn: \"%s\"\n", entry.first.c_str());
                }
                if (entry.second.targetCount <= entry.second.count ){
                    break;
                }
            }
        }
    }
}


const Populator::Entry &Populator::getEntry(std::string type) const {
    auto itr = entries.find(type);
    if (itr != entries.end()){
        return entries.at(type);
    }

    throw std::runtime_error("Couldn't find populator entry "+type);
}

void Populator::changeCount(std::string type, int deltaCount) {
    auto itr = entries.find(type);
    if (itr != entries.end()){
        entries.at(type).count += deltaCount;
    }
}

void Populator::entryEnabled(std::string type, bool enabled) {
    auto itr = entries.find(type);
    if (itr != entries.end()){
        itr->second.enabled = enabled;
    }
}
