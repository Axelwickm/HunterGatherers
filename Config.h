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
    float mushroomEnergy;

    unsigned receptorCount;
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
    bool showWorldObjectBounds{};
    bool showQuadtree{};
    bool showQuadtreeEntities{};
    bool showVision{};
};

struct Controls {
    sf::Keyboard::Key pause;// = sf::Keyboard::Space;
    sf::Keyboard::Key close;// = sf::Keyboard::Escape;

    sf::Keyboard::Key up;// = sf::Keyboard::Up;
    float upAmount;
    sf::Keyboard::Key down;// = sf::Keyboard::Down;;
    float downAmount;
    sf::Keyboard::Key left;// = sf::Keyboard::Left;
    float leftAmount;
    sf::Keyboard::Key right;// = sf::Keyboard::Right;
    float rightAmount;

    sf::Keyboard::Key slowDown;// = sf::Keyboard::Comma;
    sf::Keyboard::Key speedUp;// = sf::Keyboard::Period;
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
