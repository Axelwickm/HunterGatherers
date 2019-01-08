//
// Created by Axel on 2018-11-23.
//

#include "World.h"
#include "BouncingBall.h"
#include "Mushroom.h"
#include "Heart.h"

#include "PerlinNoise/PerlinNoise.hpp"

std::mt19937 World::randomEngine = std::mt19937(GeneralSettings::seed++);

World::World(sf::RenderWindow *window, OpenCL_Wrapper *openCL_wrapper, const WorldOptions& options):
window(window), dimensions(options.dimensions), openCL_wrapper(openCL_wrapper),
populator(this), quadtree(Quadtree<float>(sf::Vector2<float>(0, 0), dimensions)) {
    quadtree.setLimit(options.quadtreeLimit);
    populator.addEntries(options.populatorEntries);
    generateTerrain(options);

}

void World::generateTerrain(const WorldOptions &options) {
    float f = 10;
    siv::PerlinNoise perlinNoise1;
    siv::PerlinNoise perlinNoise2;
    sf::Image background;
    background.create(options.terrainSquare, options.terrainSquare, sf::Color::Black);
    for (unsigned x = 0; x < background.getSize().x; x++){
        for (unsigned y = 0; y < background.getSize().y; y++){
            double n1 = perlinNoise1.octaveNoise((float) x / (float) background.getSize().x * f, (float) y / (float) background.getSize().y * f, 20) + 1;
            double n2 = perlinNoise2.octaveNoise((float) x / (float) background.getSize().x * f * 0.3, (float) y / (float) background.getSize().y * f * 0.3, 2);
            background.setPixel(x, y, sf::Color( 20 + n1*2, 30 + n1 * 15, 20 + n1*2));
            if (n2 < -0.1){
                background.setPixel(x, y, sf::Color(10 + n2*2, 14 + n2*3, 10 + n2*2));
            }
        }
    }

    terrainTexture.loadFromImage(background);
    terrain = sf::Sprite(terrainTexture);
    terrain.setScale(dimensions.x / options.terrainSquare, dimensions.y / options.terrainSquare);
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
    window->draw(terrain);

    for (auto &object : objects) {
        object->draw(window, deltaTime);
    }

    if (RenderSettings::showQuadtree){
        quadtree.draw(window, RenderSettings::showQuadtreeEntities);
    }
}

bool World::addObject(std::shared_ptr<WorldObject> worldObject) {

    if (worldObject->isCollider()){
        if (!quadtree.add(worldObject)){
            return false;
        }
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
    bool success = worldObject->isCollider() ? quadtree.remove(worldObject.get()) : true;
    if (success){
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

void World::reproduce(Agent &a) {
    auto agent = std::make_shared<Agent>(a);
    agent->setQuadtree(&quadtree, agent);
    agent->getGenes()->mutate(0.1);
    agent->setEnergy(a.getEnergy()/2);
    a.setEnergy(a.getEnergy()/2);
    addObject(agent);
    addObject(std::make_shared<Heart>(this, a.getPosition()));
}


