//
// Created by Axel on 2018-11-21.
//

#include "Agent.h"
#include "World.h"

Agent::Agent(World* world, sf::Vector2f position) : WorldObject(world, position) {
    setAccelerationFactor(0.25);

    r.setSize(sf::Vector2f(10, 10));
    r.setOrigin(5, 5);
    r.setFillColor(sf::Color::Red);
}

void Agent::update(float deltaTime) {
    WorldObject::update(deltaTime);
}

void Agent::draw(sf::RenderWindow *window, float deltaTime) {
    r.setPosition(getPosition());
    window->draw(r);
}


