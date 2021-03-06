//
// Created by Axel on 2018-11-23.
//

#include <memory>
#include <set>
#include <deque>

#ifndef HUNTERGATHERERS_WORLD_H
#define HUNTERGATHERERS_WORLD_H

#include <SFML/Graphics.hpp>
#include "Quadtree.h"
#include "WorldObject.h"
#include "Config.h"
#include "OpenCL_Wrapper.h"
#include "Populator.h"

struct WorldStatistics {
    float timestamp = 0;
    std::size_t populationCount = 0;
    std::size_t mushroomCount = 0;
    unsigned lowestGeneration = 0;
    unsigned highestGeneration = 0;

    struct ColorValue {
        explicit operator float() const {
            return value;
        }
        sf::Color color;
        float value{};
    };

    std::vector<ColorValue> generation;
    std::vector<ColorValue> perceptrons;
    std::vector<ColorValue> age;
    std::vector<ColorValue> children;
    std::vector<ColorValue> murders;
    std::vector<ColorValue> energy;
    std::vector<ColorValue> mushrooms;
    std::vector<ColorValue> speed;
};

class World {
public:
    World(Config &config, sf::RenderWindow *window, OpenCL_Wrapper *openCL_wrapper);

    OpenCL_Wrapper *getOpenCL_wrapper() const;

    void update(float deltaTime);
    void draw(float deltaTime);

    bool addObject(std::shared_ptr<WorldObject> worldObject);
    bool removeObject(std::shared_ptr<WorldObject> worldObject, bool performImmediately = true);
    void performDeletions();
    bool spawn(std::string type);
    bool agentSpawning;

    void reproduce(Agent &a);

    Config &getConfig();
    const Populator &getPopulator() const;

    const std::set<std::shared_ptr<Agent>> &getAgents() const;
    const std::set<std::shared_ptr<WorldObject>> &getObjects() const;

    const std::deque<WorldStatistics> &getHistoricalStatistics() const;
    void clearStatistics();

    const float getHistoryFrequency() const;

    const sf::RenderWindow *getWindow() const;

    const sf::Vector2f &getDimensions() const;
    const Quadtree<float> &getQuadtree() const;


private:
    float worldTime;
    Config& config;
    sf::RenderWindow *window;
    const sf::Vector2f dimensions;
    sf::Texture terrainTexture;
    sf::Sprite terrain;

    std::list<std::shared_ptr<WorldObject>> deletionList;

    Populator populator;
    std::set<std::shared_ptr<Agent>> agents;
    std::set<std::shared_ptr<WorldObject>> objects;
    std::deque<WorldStatistics> historicalStatistics;
    const float historyFrequency;

    Quadtree<float> quadtree;
    OpenCL_Wrapper *openCL_wrapper;
    std::mt19937 randomEngine;

    void generateTerrain();

    void updateStatistics();
};


#endif //HUNTERGATHERERS_WORLD_H
