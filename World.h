//
// Created by Axel on 2018-11-23.
//

#include <memory>
#include <set>
#include <random>
#include <chrono>

#ifndef HUNTERGATHERERS_WORLD_H
#define HUNTERGATHERERS_WORLD_H

#include <SFML/Graphics.hpp>
#include "Quadtree.h"
#include "WorldObject.h"
#include "Config.h"

class World {
public:
    World(sf::RenderWindow* window, sf::Vector2f dimensions);

    void update(float deltaTime);
    void draw(float deltaTime);

    bool addObject(std::shared_ptr<WorldObject> worldObject);
    bool removeObject(WorldObject* worldObject);

private:
    sf::RenderWindow* window;
    const sf::Vector2f dimensions;
    Quadtree<float> quadtree;

    std::set<std::shared_ptr<WorldObject> > objects;

};


#endif //HUNTERGATHERERS_WORLD_H
