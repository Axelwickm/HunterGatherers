//
// Created by Axel on 2018-11-23.
//

#include <SFML/Graphics.hpp>
#include "World.h"

namespace GeneralSettings {
    sf::Vector2u windowSize(1920, 1080);
    WorldOptions options = {
        dimensions : sf::Vector2f(2000, 2000),
        quadtreeLimit : 30,
        populatorEntries:{{
            .type = "Agent",
            .count = 0,
            .targetCount = 25,
            .rate = 3
        },{
            .type = "Mushroom",
            .count = 0,
            .targetCount = 50,
            .rate = 3
        },{
            .type = "Bouncing ball",
            .count = 0,
            .targetCount = 0,
            .rate = 0.75
        }}
    };
}

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
