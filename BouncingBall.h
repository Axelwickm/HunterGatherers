//
// Created by Axel on 2018-11-22.
//

#ifndef FAMILYISEVERYTHING_BOUNCINGBALLS_H
#define FAMILYISEVERYTHING_BOUNCINGBALLS_H

#include "WorldObject.h"

#include <SFML/Graphics.hpp>


class BouncingBall : public WorldObject {
public:
    BouncingBall(sf::Vector2f position, float radius, sf::Vector2f worldBounds);

    void setVelocity(sf::Vector2f velocity);

    void update(float deltaTime) override;

    void draw(sf::RenderWindow *window, float deltaTime) override;

    float getRadius();

private:
    float radius;
    sf::CircleShape c;
    sf::Vector2f worldBounds;
    sf::Vector2f velocity;
};


#endif //FAMILYISEVERYTHING_BOUNCINGBALLS_H
