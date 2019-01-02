//
// Created by Axel on 2018-11-22.
//

#include "BouncingBall.h"

#include "Quadtree.h"
#include "World.h"

BouncingBall::BouncingBall(World* world, sf::Vector2f position, float radius) : WorldObject("BouncingBall",
                                                                                            world, position) {
    this->radius = radius;
    setBounds(sf::IntRect(-radius, -radius, radius, radius));

    c.setRadius(radius);
    c.setFillColor(sf::Color::Magenta);
    c.setOrigin(c.getRadius(), c.getRadius());
}

void BouncingBall::update(float deltaTime) {
    WorldObject::update(deltaTime);

    auto newPos = getPosition();
    if (world->getDimensions().x < newPos.x + radius || newPos.x < radius) {
        velocity.x *= -1.f;
    }

    if (world->getDimensions().y < newPos.y + radius || newPos.y < radius) {
        velocity.y *= -1.f;
    }

    if (quadtree != nullptr) {
        auto nl = quadtree->searchNear(newPos, 4*radius);
        c.setFillColor(sf::Color(20, 20, 20));
        for (auto &n : nl) {
            if (n.get() != this && typeid(*(n.get())) == typeid(BouncingBall)) {
                sf::Vector2f v = n->getPosition() - position;
                if (sqrtf(v.x * v.x + v.y * v.y) < ((BouncingBall *) n.get())->getRadius() + radius) {
                    //c.setFillColor(sf::Color::Green);
                }
            }
            else if (n.get() != this){
                sf::FloatRect a(position.x-radius, position.y-radius, 2*radius, 2*radius);

                sf::FloatRect b(n->getPosition().x + n->getBounds().left,
                    n->getPosition().y + n->getBounds().top,
                    n->getBounds().width - n->getBounds().left,
                    n->getBounds().height - n->getBounds().top);
                c.setFillColor(sf::Color(100, 100, 100));
                if (boxesIntersect(a, b)){
                    c.setFillColor(sf::Color::Cyan);
                }

            }

        }
    }


}

void BouncingBall::draw(sf::RenderWindow *window, float deltaTime) {
    c.setPosition(getPosition());
    window->draw(c);
    WorldObject::draw(window, deltaTime);
}

float BouncingBall::getRadius() {
    return radius;
}
