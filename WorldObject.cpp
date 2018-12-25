//
// Created by Axel on 2018-11-21.
//

#include "WorldObject.h"
#include "Quadtree.h"
#include "World.h"

WorldObject::WorldObject(World* world, sf::Vector2f position) {
    this->position = position;
    this->quadtree = nullptr;
    this->world = world;
    this->velocity = sf::Vector2f(0, 0);
    accelerationFactor = 1.f;
}

std::shared_ptr<WorldObject> WorldObject::getSharedPtr() {
    return std::shared_ptr<WorldObject>(me);
}

void WorldObject::setQuadtree(Quadtree<float> *quadtree, std::weak_ptr<WorldObject> object) {
    this->quadtree = quadtree;
    this->me = object;
}

Quadtree<float> *WorldObject::getQuadtree() {
    return quadtree;
}

void WorldObject::update(float deltaTime) {
    sf::Vector2f old = position;
    update(deltaTime, old);
}

void WorldObject::update(float deltaTime, sf::Vector2f oldPosition) {
    velocity *= powf(accelerationFactor, deltaTime);
    position += velocity * deltaTime;
    position = sf::Vector2f((float) fmin(position.x, world->getDimensions().x), (float) fmin(position.y, world->getDimensions().y));
    position = sf::Vector2f((float) fmax(position.x, 1), (float) fmax(position.y, 1));
    if (quadtree != nullptr) {
        quadtree->move(oldPosition, this);
    }

}

const sf::Vector2f &WorldObject::getPosition() const {
    return position;
}

void WorldObject::setPosition(const sf::Vector2f &position) {
    WorldObject::position = position;
}


const sf::Vector2f &WorldObject::getVelocity() const {
    return velocity;
}

void WorldObject::setVelocity(const sf::Vector2f &velocity) {
    WorldObject::velocity = velocity;
}

float WorldObject::getAccelerationFactor() const {
    return accelerationFactor;
}

void WorldObject::setAccelerationFactor(float accelerationFactor) {
    WorldObject::accelerationFactor = accelerationFactor;
}



