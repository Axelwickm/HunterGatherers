//
// Created by Axel on 2018-11-21.
//

#ifndef FAMILYISEVERYTHING_AGENT_H
#define FAMILYISEVERYTHING_AGENT_H

#include "WorldObject.h"

#include <SFML/Graphics.hpp>

class Agent : public WorldObject {
public:
    Agent(World* world, sf::Vector2f position);

    void update(float deltaTime) override;
    void draw(sf::RenderWindow *window, float deltaTime) override;

private:
    sf::RectangleShape r;
};


#endif //FAMILYISEVERYTHING_AGENT_H
