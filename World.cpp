//
// Created by Axel on 2018-11-23.
//

#include "World.h"
#include "Agent.h"
#include "BouncingBall.h"
#include "Populator.h"
#include "Mushroom.h"

World::World(sf::RenderWindow *window, OpenCL_Wrapper *openCL_wrapper, const WorldOptions& options):
window(window), dimensions(options.dimensions), openCL_wrapper(openCL_wrapper),
populator(this), quadtree(Quadtree<float>(sf::Vector2<float>(0, 0), dimensions)) {
    quadtree.setLimit(options.quadtreeLimit);

    long long int now = std::chrono::time_point_cast<std::chrono::microseconds>(
            std::chrono::system_clock::now()).time_since_epoch().count();

    srand(now);

    populator.addEntries(options.populatorEntries);

}

void World::update(float deltaTime) {
    openCL_wrapper->clFinishAll(); // More optimized to have this here?

    populator.populate(deltaTime);

    // World updates
    for (auto &object : objects) {
        object->update(deltaTime);
    }

    // AI updates
    for (auto& agent : agents){
        agent->updatePercept(deltaTime);
        openCL_wrapper->think(agent, agent->getPercept());
    }

    performDeletions();

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
        agents.insert(std::dynamic_pointer_cast<Agent>(worldObject));
        openCL_wrapper->addAgent((Agent*) worldObject.get());
    }

    populator.changeCount(worldObject->type, 1);

    return true;

}

bool World::removeObject(std::shared_ptr<WorldObject> worldObject, bool performImmediately) {
    if (!performImmediately){
        deletionList.push_back(worldObject);
        return true;
    }

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

void World::performDeletions() {
    for (auto& worldObject : deletionList){
        removeObject(worldObject, true);
    }

    deletionList.clear();
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

bool World::reproduce(Agent& a) {
    auto agent = std::make_shared<Agent>(a);
    agent->getGenes()->mutate(0.1);
    agent->setEnergy(a.getEnergy()/2);
    a.setEnergy(a.getEnergy()/2);
    return addObject(agent);
}


