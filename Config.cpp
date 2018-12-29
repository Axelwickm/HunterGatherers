//
// Created by Axel on 2018-11-23.
//

#include <SFML/Graphics.hpp>

namespace RenderSettings {
    bool showWorldObjectBounds = true;
    bool showQuadtree = true;
    bool showQuadtreeEntities = true;
    bool showVision = true;

};

namespace Controls {
    sf::Keyboard::Key pause = sf::Keyboard::Space;
    sf::Keyboard::Key close = sf::Keyboard::Escape;

    float scrollFactor = 0.05;
};
