//
// Created by Axel on 2018-11-23.
//

#include "World.h"
#include "Agent.h"
#include "BouncingBall.h"
#include "Populator.h"
#include "Mushroom.h"

World::World(sf::RenderWindow *window, sf::Vector2f dimensions, OpenCL_Wrapper *openCL_wrapper):
window(window), dimensions(dimensions), openCL_wrapper(openCL_wrapper),
populator(this), quadtree(Quadtree<float>(sf::Vector2<float>(0, 0), dimensions)) {
    quadtree.setLimit(30);

    long long int now = std::chrono::time_point_cast<std::chrono::microseconds>(
            std::chrono::system_clock::now()).time_since_epoch().count();

    srand(now);

    populator.addEntry("Agent", {
            .count = 0,
            .targetCount = 25,
            .rate = 3
    });

    populator.addEntry("Bouncing ball", {
            .count = 0,
            .targetCount = 0,
            .rate = 0.75
    });

    populator.addEntry("Mushroom", {
            .count = 0,
            .targetCount = 50,
            .rate = 3
    });



}

void World::update(float deltaTime) {
    populator.populate(deltaTime);

    openCL_wrapper->clFinishAll(); // More optimized to have this here?

    // World updates
    for (auto &object : objects) {
        object->update(deltaTime);
    }

    // AI updates
    for (auto& agent : agents){
        agent->updatePercept(deltaTime);
        openCL_wrapper->think(agent.get(), agent->getPercept());
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
        openCL_wrapper->addAgent((Agent*) worldObject.get());
    }

    return true;

}

bool World::removeObject(std::shared_ptr<WorldObject> worldObject) {
    if (quadtree.remove(worldObject.get())){
        if (typeid(*worldObject.get()) == typeid(Agent)){
            openCL_wrapper->removeAgent((Agent*) worldObject.get());
            agents.erase(std::dynamic_pointer_cast<Agent>(worldObject));
        }

        objects.erase(worldObject);
        populator.changeCount(worldObject->type, -1);
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

bool World::spawn(std::string type) {
    if (type == "Agent"){
        sf::Vector2<float> position(rand() % ((int) dimensions.x - 50) + 25, rand() % ((int) dimensions.y - 50) + 25);
        auto agent = std::make_shared<Agent>(this, position, (float) rand()/RAND_MAX*360.f);
        agent->setVelocity(sf::Vector2f(0, 0));
        agents.insert(agent);
        return addObject(agent);
    }
    else if (type == "Mushroom"){
        sf::Vector2<float> position(rand() % ((int) dimensions.x - 50) + 25, rand() % ((int) dimensions.y - 50) + 25);
        std::shared_ptr<Mushroom> w(new Mushroom(this, position));
        return addObject(w);
    }
    else if (type == "Bouncing ball"){
        sf::Vector2<float> position(rand() % ((int) dimensions.x - 50) + 25, rand() % ((int) dimensions.y - 50) + 25);
        std::shared_ptr<BouncingBall> w(new BouncingBall(this, position, 10.f));
        w->setVelocity({(float) rand() / RAND_MAX * 50.f - 25.f, (float) rand() / RAND_MAX * 50.f - 25.f});
        return addObject(w);
    }
    return false;
}

bool World::reproduce(Agent *a) {
    return false;
}

