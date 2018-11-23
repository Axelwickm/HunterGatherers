//
// Created by Axel on 2018-11-22.
//

#include "BouncingBall.h"

#include "Quadtree.h"

BouncingBall::BouncingBall(sf::Vector2f position, float radius, sf::Vector2f worldBounds) : WorldObject(position) {
    this->radius = radius;
    this->worldBounds = worldBounds;

    c.setRadius(radius);
    c.setFillColor(sf::Color::Magenta);
    c.setOrigin(c.getRadius(), c.getRadius());
}

void BouncingBall::update(float deltaTime) {
    sf::Vector2f old = position;

    position += velocity * deltaTime;
    auto newPos = getPosition();
    if (worldBounds.x < newPos.x + radius || newPos.x < radius) {
        velocity.x *= -1.f;
    }

    if (worldBounds.y < newPos.y + radius || newPos.y < radius) {
        velocity.y *= -1.f;
    }

    if (quadtree != nullptr) {
        auto nl = quadtree->searchNear(newPos, radius);
        c.setFillColor(sf::Color::Magenta);
        for (auto &n : nl) {
            if (n.get() != this) {
                sf::Vector2f v = n->getPosition() - position;
                if (sqrtf(v.x * v.x + v.y * v.y) < ((BouncingBall *) n.get())->getRadius() + radius) {
                    c.setFillColor(sf::Color::Green);
                }
            }
        }

        quadtree->move(old, this);
    }
}

void BouncingBall::draw(sf::RenderWindow *window, float deltaTime) {
    c.setPosition(getPosition());
    window->draw(c);

}

void BouncingBall::setVelocity(sf::Vector2f velocity) {
    this->velocity = velocity;
}

float BouncingBall::getRadius() {
    return radius;
}
