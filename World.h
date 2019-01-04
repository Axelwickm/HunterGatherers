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
#include "Populator.h"


class World {
public:
    World(sf::RenderWindow* window, sf::Vector2f dimensions, OpenCL_Wrapper *openCL_wrapper);

    OpenCL_Wrapper *getOpenCL_wrapper() const;

    void update(float deltaTime);
    void draw(float deltaTime);

    bool addObject(std::shared_ptr<WorldObject> worldObject);
    bool removeObject(std::shared_ptr<WorldObject> worldObject);
    bool spawn(std::string type);

    bool reproduce(Agent* a);


    const sf::RenderWindow *getWindow() const;

    const sf::Vector2f &getDimensions() const;

    const Quadtree<float> &getQuadtree() const;

private:
    sf::RenderWindow* window;
    const sf::Vector2f dimensions;

    Populator populator;
    std::set<std::shared_ptr<Agent>> agents;
    std::set<std::shared_ptr<WorldObject> > objects;

    Quadtree<float> quadtree;
    OpenCL_Wrapper *openCL_wrapper;

};


#endif //HUNTERGATHERERS_WORLD_H
