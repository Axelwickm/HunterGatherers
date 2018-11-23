//
// Created by Axel on 2018-11-21.
//

#include "WorldObject.h"

WorldObject::WorldObject(sf::Vector2f position) {
    this->position = position;
    this->quadtree = nullptr;
}

std::shared_ptr<WorldObject> WorldObject::getSharedPtr() {
    return std::shared_ptr<WorldObject>(me);
}

sf::Vector2f WorldObject::getPosition() { return position; }

void WorldObject::setPosition(sf::Vector2f position) { this->position = position; }

void WorldObject::setQuadtree(Quadtree<float> *quadtree, std::weak_ptr<WorldObject> object) {
    this->quadtree = quadtree;
    this->me = object;
}

Quadtree<float> *WorldObject::getQuadtree() {
    return quadtree;
}


