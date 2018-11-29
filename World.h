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
#include "OpenCL_Wrapper.h"

class World {
public:
    World(sf::RenderWindow* window, sf::Vector2f dimensions, OpenCL_Wrapper *openCL_wrapper);

    OpenCL_Wrapper *getOpenCL_wrapper() const;

    void update(float deltaTime);
    void draw(float deltaTime);

    bool addObject(std::shared_ptr<WorldObject> worldObject);
    bool removeObject(WorldObject* worldObject);


    const sf::RenderWindow *getWindow() const;

    const sf::Vector2f &getDimensions() const;

    const Quadtree<float> &getQuadtree() const;

private:
    sf::RenderWindow* window;
    const sf::Vector2f dimensions;

    std::shared_ptr<Agent> agent;

    Quadtree<float> quadtree;
    OpenCL_Wrapper *openCL_wrapper;

    std::set<std::shared_ptr<WorldObject> > objects;

};


#endif //HUNTERGATHERERS_WORLD_H
