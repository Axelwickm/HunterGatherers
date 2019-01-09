//
// Created by Axel on 2018-11-23.
//

#include <SFML/Graphics.hpp>
#include <chrono>
#include "World.h"

namespace GeneralSettings {
    long unsigned int seed = static_cast<long unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    sf::Vector2u windowSize(1920, 1080);
    WorldOptions options = {
        dimensions : sf::Vector2f(2500, 2500),
        terrainSquare : 40,
        quadtreeLimit : 30,
        populatorEntries:{{
            .type = "Agent",
            .count = 0,
            .targetCount = 65,
            .rate = 5
        },{
            .type = "Mushroom",
            .count = 0,
            .targetCount = 150,
            .rate = 5
        },{
            .type = "BouncingBall",
            .count = 0,
            .targetCount = 0,
            .rate = 0.75
        }}
    };
}

namespace RenderSettings {
    bool showWorldObjectBounds = false;
    bool showQuadtree = false;
    bool showQuadtreeEntities = false;
    bool showVision = true;

};

namespace Controls {
    sf::Keyboard::Key pause = sf::Keyboard::Space;
    sf::Keyboard::Key close = sf::Keyboard::Escape;

    sf::Keyboard::Key up = sf::Keyboard::Up;
    float upAmount = -15;
    sf::Keyboard::Key down = sf::Keyboard::Down;
    float downAmount = 15;
    sf::Keyboard::Key left = sf::Keyboard::Left;
    float leftAmount = -15;
    sf::Keyboard::Key right = sf::Keyboard::Right;
    float rightAmount = 15;

    sf::Keyboard::Key slowDown = sf::Keyboard::Comma;
    sf::Keyboard::Key speedUp = sf::Keyboard::Period;

    float timeFactorDelta = 0.1;
    float timeFactorMax = 20;
    float scrollFactor = 0.05;
};
