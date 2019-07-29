//
// Created by Axel on 2018-11-23.
//

#ifndef HUNTERGATHERERS_CONFIG_H
#define HUNTERGATHERERS_CONFIG_H

#include <SFML/Graphics.hpp>
#include "json/json.hpp"
#include "Populator.h"

struct WorldSettings {
    sf::Vector2f dimensions;
    unsigned terrainSquare{};
    float quadtreeLimit{};
    std::map<std::string, Populator::Entry> populatorEntries{};
};

struct AgentSettings {
    float mass;
    float friction;
    float maxSpeed;
    float turnFactor;
    float punchTime;
    float actionCooldown;

    float energyLossRate;
    float movementEnergyLoss;
    float punchEnergy;
    float punchDamage;
    float mushroomEnergy;
    float maxEnergy;
    unsigned maxMushroomCount;

    bool canReproduce;
    bool canWalk;
    bool canTurn;
    bool canEat;
    bool canPlace;
    bool canPunch;

    unsigned memory;
    float memoryReactivity;

    bool perceiveCollision;
    unsigned receptorCount;
    bool perceiveColor;
    bool perceiveEnergyLevel;
    bool perceiveMushroomCount;

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

    bool graphLine{};
        bool graphPopulation{};
        bool graphMeanGeneration{};
        bool graphMeanPerceptrons{};
        bool graphMeanAge{};
        bool graphMeanChildren{};
        bool graphMeanMurders{};
        bool graphMeanEnergy{};
        bool graphMeanMushrooms{};
        bool graphMeanSpeed{};

    bool graphSpectrogram{};
        bool graphGeneration{};
        bool graphPerceptrons{};
        bool graphAge{};
        bool graphEnergy{};
        bool graphChildren{};
        bool graphMurders{};
        bool graphMushrooms{};
        bool graphSpeed{};

    unsigned bins{};
    bool showDebug{};
    bool showWorldObjectBounds{};
    bool showQuadtree{};
    bool showQuadtreeEntities{};
    bool showVision{};
    bool renderOnlyAgents{};
    bool visualizeGeneration{};
    bool visualizeAge{};
    bool visualizeMushrooms{};
    bool visualizeChildren{};
    bool visualizeMurders{};
    bool visualizeColor{};
};

struct Controls {
    sf::Keyboard::Key pause;
    sf::Keyboard::Key close;
    sf::Keyboard::Key showDebug;
    sf::Keyboard::Key clearStats;

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
    float timeFactorInitial;
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

    bool shouldReload;

    void loadConfigFromFile(const std::string &filename);

    sf::Keyboard::Key findKeyCode(std::string key);

};


#endif //HUNTERGATHERERS_CONFIG_H
