//
// Created by Axel on 2018-11-23.
//

#ifndef HUNTERGATHERERS_CONFIG_H
#define HUNTERGATHERERS_CONFIG_H

#include <SFML/Graphics.hpp>

struct WorldOptions;

namespace GeneralSettings {
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
    extern float scrollFactor;
};


#endif //HUNTERGATHERERS_CONFIG_H
