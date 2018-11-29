//
// Created by Axel on 2018-11-23.
//

#include "World.h"
#include "Agent.h"
#include "BouncingBall.h"

World::World(sf::RenderWindow *window, sf::Vector2f dimensions, OpenCL_Wrapper *openCL_wrapper):
window(window), dimensions(dimensions), openCL_wrapper(openCL_wrapper), quadtree(Quadtree<float>(sf::Vector2<float>(0, 0), dimensions)) {
    quadtree.setLimit(30);

    long long int now = std::chrono::time_point_cast<std::chrono::microseconds>(
            std::chrono::system_clock::now()).time_since_epoch().count();

    srand(now);


    for (int i = 0; i < 100; i++) {
        sf::Vector2<float> position(rand() % ((int) dimensions.x - 50) + 25, rand() % ((int) dimensions.y - 50) + 25);
        std::shared_ptr<BouncingBall> w(new BouncingBall(this, position, 10.f));
        w->setVelocity({(float) rand() / RAND_MAX * 50.f - 25.f, (float) rand() / RAND_MAX * 50.f - 25.f});
        addObject(w);
    }

    agent = std::shared_ptr<Agent>(new Agent(this, sf::Vector2f(100, 100)));
    agent->setVelocity(sf::Vector2f(100, 10));
    addObject(agent);
}

void World::update(float deltaTime) {
    // AI updates
    agent->updatePercept(deltaTime);
    openCL_wrapper->think(agent.get(), agent->percept);

    // World updates
    for (auto &object : objects) {
        object->update(deltaTime);
    }
}

void World::draw(float deltaTime) {
    for (auto &object : objects) {
        object->draw(window, deltaTime);
    }

    if (RenderSettings::showQuadtree){
        quadtree.draw(window, RenderSettings::showQuadtreeEntities);
    }
}

bool World::addObject(std::shared_ptr<WorldObject> worldObject) {
    if (!quadtree.add(worldObject)){
        return false;
    }
    worldObject->setQuadtree(&quadtree, worldObject);
    objects.insert(worldObject);

    if (typeid(*worldObject.get()) == typeid(Agent)){

        openCL_wrapper->addAgent(std::weak_ptr<Agent>(std::dynamic_pointer_cast<Agent>(worldObject)));
    }

    return true;

}

bool World::removeObject(WorldObject* worldObject) {
    if (quadtree.remove(worldObject)){
        objects.erase(std::shared_ptr<WorldObject>(worldObject));
        return true;
    }
    return false;
}

const sf::RenderWindow *World::getWindow() const {
    return window;
}

const sf::Vector2f &World::getDimensions() const {
    return dimensions;
}

const Quadtree<float> &World::getQuadtree() const {
    return quadtree;
}

OpenCL_Wrapper *World::getOpenCL_wrapper() const {
    return openCL_wrapper;
}

