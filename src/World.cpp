//
// Created by Axel on 2018-11-23.
//

#include "World.h"
#include "BouncingBall.h"
#include "Mushroom.h"
#include "Heart.h"

#include "PerlinNoise/PerlinNoise.hpp"

World::World(Config &config, sf::RenderWindow *window, OpenCL_Wrapper *openCL_wrapper) :
worldTime(0), config(config), window(window), dimensions(config.world.dimensions), openCL_wrapper(openCL_wrapper),
populator(this), quadtree(Quadtree<float>(sf::Vector2<float>(0, 0), dimensions)),
historyFrequency(3) {
    randomEngine = std::mt19937(config.seed++);
    quadtree.setLimit(config.world.quadtreeLimit);
    generateTerrain();

    agentSpawning = true;
}

void World::generateTerrain() {
    float f = 10;
    siv::PerlinNoise perlinNoise1;
    siv::PerlinNoise perlinNoise2;
    perlinNoise1.reseed(config.seed++);
    perlinNoise2.reseed(config.seed++);
    sf::Image background;
    background.create(config.world.terrainSquare, config.world.terrainSquare, sf::Color::Black);
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
    terrain.setScale(dimensions.x / config.world.terrainSquare, dimensions.y / config.world.terrainSquare);
}

void World::update(float deltaTime) {
    worldTime += deltaTime;
    openCL_wrapper->clFinishAll(); // More optimized to have this here?

    populator.entryEnabled("Agent", agentSpawning);

    populator.populate(deltaTime);


    // World updates
    for (auto &object : objects) {
        object->update(deltaTime);
    }

    performDeletions();

    // AI updates
    for (auto &agent : agents) {
        agent->updatePercept(deltaTime);
        openCL_wrapper->think(agent, agent->getPercept());
    }

    updateStatistics();


}

void World::draw(float deltaTime) {
    if (!config.render.renderOnlyAgents){
        window->draw(terrain);
    }

    if (!config.render.renderOnlyAgents){
        for (auto &object : objects) {
            object->draw(window, deltaTime);
        }
    }
    else {
        for (auto &agent : agents){
            agent->draw(window, deltaTime);
        }
    }

    if (config.render.showQuadtree){
        quadtree.draw(window, config.render.showQuadtreeEntities);
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
    for (auto &worldObject : deletionList) {
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
        auto agent = std::make_shared<Agent>(config.agents, this, position, orientation);
        agent->setVelocity(sf::Vector2f(0, 0));
        return addObject(agent);
    }
    else if (type == "Mushroom"){
        sf::Vector2<float> position(spawnX(randomEngine), spawnY(randomEngine));
        std::shared_ptr<Mushroom> w(new Mushroom(this, position, config));
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
    auto agent = std::make_shared<Agent>(a, config.agents.mutation);
    agent->setGeneration(agent->getGeneration()+1);
    agent->setQuadtree(&quadtree, agent);

    agent->setOrientation(std::uniform_real_distribution<float>(0, 360)(randomEngine));
    float totalEnergy = agent->getEnergy();
    a.setEnergy(totalEnergy*config.agents.energyToParent);
    agent->setEnergy(totalEnergy*config.agents.energyToChild);
    a.setChildCount(a.getChildCount()+1);
    addObject(agent);
    addObject(std::make_shared<Heart>(this, a.getPosition()));
    printf("Reproduced to gen %u : %s -> %s\n", agent->getGeneration(), a.getName().c_str(), agent->getName().c_str());
}

Config & World::getConfig() {
    return config;
}

const std::set<std::shared_ptr<Agent>> &World::getAgents() const {
    return agents;
}

const std::set<std::shared_ptr<WorldObject>> &World::getObjects() const {
    return objects;
}

void World::updateStatistics() {
    WorldStatistics statistics;
    if (!historicalStatistics.empty())
        if (worldTime - historicalStatistics.back().timestamp <= historyFrequency)
            return; // Not time yet

    statistics.timestamp = worldTime;
    statistics.populationCount = agents.size();
    statistics.mushroomCount = populator.getEntry("Mushroom").count;

    statistics.generation.reserve(agents.size());
    statistics.perceptrons.reserve(agents.size());
    statistics.age.reserve(agents.size());
    statistics.children.reserve(agents.size());
    statistics.murders.reserve(agents.size());
    statistics.energy.reserve(agents.size());
    statistics.mushrooms.reserve(agents.size());
    statistics.speed.reserve(agents.size());

    statistics.lowestGeneration = std::numeric_limits<unsigned>::max();
    statistics.highestGeneration = 0;
    for (auto& agent : agents){
        unsigned g = agent->getGeneration();

        if (g < statistics.lowestGeneration){
            statistics.lowestGeneration = g;
        }
        if (statistics.highestGeneration < g){
            statistics.highestGeneration = g;
        }

        statistics.generation.push_back({agent->getColor(), (float) agent->getGeneration()});
        statistics.perceptrons.push_back({agent->getColor(), (float) agent->getNetworkStatistics().perceptronCount});
        statistics.age.push_back({agent->getColor(), agent->getAge()});
        statistics.children.push_back({agent->getColor(), (float) agent->getChildCount()});
        statistics.murders.push_back({agent->getColor(), (float) agent->getMurderCount()});
        statistics.energy.push_back({agent->getColor(), std::max(0.f, agent->getEnergy())});
        statistics.mushrooms.push_back({agent->getColor(), (float) agent->getInventory().mushrooms});
        if (agent->getSpeed() < 400)
            statistics.speed.push_back({agent->getColor(), agent->getSpeed()});
    }
    historicalStatistics.push_back(statistics);
}

const std::deque<WorldStatistics> &World::getHistoricalStatistics() const {
    return historicalStatistics;
}

void World::clearStatistics() {
    printf("Clearing all statistics\n");
    historicalStatistics.clear();
}

const float World::getHistoryFrequency() const {
    return historyFrequency;
}

const Populator &World::getPopulator() const {
    return populator;
}
