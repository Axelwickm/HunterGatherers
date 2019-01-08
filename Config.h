//
// Created by Axel on 2018-11-23.
//

#ifndef HUNTERGATHERERS_CONFIG_H
#define HUNTERGATHERERS_CONFIG_H

#include <SFML/Graphics.hpp>

struct WorldOptions;

namespace GeneralSettings {
    extern long unsigned int seed;
    extern sf::Vector2u windowSize;
    extern WorldOptions options;
}

namespace RenderSettings {
    extern bool showWorldObjectBounds;
    extern bool showQuadtree;
    extern bool showQuadtreeEntities;
    extern bool showVision;
};

namespace Controls {
    extern sf::Keyboard::Key pause;
    extern sf::Keyboard::Key close;

    extern sf::Keyboard::Key up;
    extern float upAmount;
    extern sf::Keyboard::Key down;
    extern float downAmount;
    extern sf::Keyboard::Key left;
    extern float leftAmount;
    extern sf::Keyboard::Key right;
    extern float rightAmount;

    extern sf::Keyboard::Key slowDown;
    extern sf::Keyboard::Key speedUp;
    extern float timeFactorDelta;
    extern float timeFactorMax;
    extern float scrollFactor;
};


#endif //HUNTERGATHERERS_CONFIG_H
