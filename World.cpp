//
// Created by Axel on 2018-11-23.
//

#include "World.h"
#include "BouncingBall.h"
#include "Mushroom.h"

std::mt19937 World::randomEngine = std::mt19937(std::random_device()());

World::World(sf::RenderWindow *window, OpenCL_Wrapper *openCL_wrapper, const WorldOptions& options):
window(window), dimensions(options.dimensions), openCL_wrapper(openCL_wrapper),
populator(this), quadtree(Quadtree<float>(sf::Vector2<float>(0, 0), dimensions)) {
    quadtree.setLimit(options.quadtreeLimit);
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
    std::uniform_real_distribution<float> spawnX(25, dimensions.x-50);
    std::uniform_real_distribution<float> spawnY(25, dimensions.y-50);


    if (type == "Agent"){
        sf::Vector2<float> position(spawnX(randomEngine), spawnY(randomEngine));
        float orientation = std::uniform_real_distribution<float>(0, 360)(randomEngine);
        auto agent = std::make_shared<Agent>(this, position, orientation);
        agent->setVelocity(sf::Vector2f(0, 0));
        return addObject(agent);
    }
    else if (type == "Mushroom"){
        sf::Vector2<float> position(spawnX(randomEngine), spawnY(randomEngine));
        std::shared_ptr<Mushroom> w(new Mushroom(this, position));
        return addObject(w);
    }
    else if (type == "BouncingBall"){
        sf::Vector2<float> position(spawnX(randomEngine), spawnY(randomEngine));
        std::shared_ptr<BouncingBall> w(new BouncingBall(this, position, 10.f));
        std::uniform_real_distribution<float> velocity(-30, 30);
        w->setVelocity({velocity(randomEngine), velocity(randomEngine)});
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


