//
// Created by axelw on 2018-12-26.
//

#include <ctime>
#include "World.h"
#include "Populater.h"

std::mt19937 Populater::randomEngine = std::mt19937(static_cast<unsigned int>(time(nullptr)));

Populater::Populater(World* world) : world(world) {}

void Populater::populate(float deltaT) {
    for (auto& entry : frequencies){
        if (entry.second.count < entry.second.targetCount){
            auto d = std::poisson_distribution(entry.second.rate*deltaT);
            int newCount = d(randomEngine);
            for (unsigned i = 0; i < newCount; i++){
                entry.second.count += world->spawn(entry.first);
            }
        }
    }
}

void Populater::addEntry(std::string type, Entry entry) {
    frequencies.insert(std::make_pair(type, entry));
}

void Populater::changeCount(std::string type, int deltaCount) {
    frequencies.at(type).count += deltaCount;
}
