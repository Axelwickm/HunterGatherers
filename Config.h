//
// Created by Axel on 2018-11-23.
//

#ifndef HUNTERGATHERERS_CONFIG_H
#define HUNTERGATHERERS_CONFIG_H

#include <SFML/Graphics.hpp>
#include "json/json.hpp"

struct WorldSettings {
    struct PopulationEntry {
        std::string type;
        unsigned count;
        unsigned targetCount;
        float rate;
    };

    sf::Vector2f dimensions;
    unsigned terrainSquare{};
    float quadtreeLimit{};
    std::vector<PopulationEntry> populatorEntries{};
};

struct AgentSettings {
    float mass;
    float friction;
    float maxSpeed;
    float turnFactor;

    float energyLossRate;
    float movementEnergyLoss;
    float mushroomEnergy;

    unsigned receptorCount;
    bool color;
    bool energyLevel;
    float FOV;
    float visibilityDistance;
    float visualReactivity;

    float mutation;
    int layersMin, layersMax;
    float biasMin, biasMax;
    float weightMin, weightMax;
    int perceptronPerLayerMin, perceptronPerLayerMax;
};


struct RenderSettings {
    sf::Vector2u windowSize;

    bool showDistribution{};
    unsigned bins{};
    bool showDebug{};
    bool showWorldObjectBounds{};
    bool showQuadtree{};
    bool showQuadtreeEntities{};
    bool showVision{};
};

struct Controls {
    sf::Keyboard::Key pause;
    sf::Keyboard::Key close;
    sf::Keyboard::Key showDebug;

    sf::Keyboard::Key up;
    float upAmount;
    sf::Keyboard::Key down;
    float downAmount;
    sf::Keyboard::Key left;
    float leftAmount;
    sf::Keyboard::Key right;
    float rightAmount;

    sf::Keyboard::Key slowDown;
    sf::Keyboard::Key speedUp;
    float timeFactorDelta;
    float timeFactorMax;
    float scrollFactor;
};


struct Config {
    long unsigned int seed{};
    WorldSettings world{};
    AgentSettings agents{};
    RenderSettings render;
    Controls controls{};

    void loadConfigFromFile(const std::string &filename);

    sf::Keyboard::Key findKeyCode(std::string key);

};


#endif //HUNTERGATHERERS_CONFIG_H
