//
// Created by Axel on 2018-11-21.
//

#ifndef FAMILYISEVERYTHING_AGENT_H
#define FAMILYISEVERYTHING_AGENT_H

#include "WorldObject.h"
#include "Gene.h"
#include "Config.h"

#include <SFML/Graphics.hpp>
#include <vector>

class Agent : public WorldObject {
public:
    Agent(const AgentSettings &settings, World *world, sf::Vector2f position, float orientation);
    Agent(const Agent &other, float mutation);

    const AgentSettings &getSettings() const;

    const std::string &getName() const;

    static void loadResources();

    MapGenes *getGenes() const;

    unsigned int getGeneration() const;
    void setGeneration(unsigned int generation);

    unsigned int getChildCount() const;
    void setChildCount(unsigned int childCount);

    unsigned int getMurderCount() const;
    void setMurderCount(unsigned int murderCount);

    unsigned int getNewBirths();
    unsigned int getNewMurders();

    void update(float deltaTime) override;
    void draw(sf::RenderWindow *window, float deltaTime) override;

    void updatePercept(float deltaTime);

    const std::vector<float> &getPercept() const;
    void setPercept(const std::vector<float> &percept);

    const std::vector<float> &getActions() const;
    void setActions(const std::vector<float> &actions);

    float getOrientation() const;
    void setOrientation(float orientation);

    float getEnergy() const;
    void setEnergy(float energy);

    const std::vector<float> &getReceptors() const;
    const std::vector<float> &getMemory() const;

    std::vector<float> getRegressionPercept(unsigned id) const;
    std::vector<float> getRegressionActions(unsigned id) const;

    void clearPath();
    void queuePathDraw();

    struct Inventory {
        unsigned mushrooms;
    };

    const Inventory &getInventory() const;
    void setInventory(const Inventory &inventory);

    struct NetworkStatistics {
        unsigned layers;
        unsigned perceptronCount;
    };

    const NetworkStatistics &getNetworkStatistics() const;
    void setNetworkStatistics(const NetworkStatistics &networkStatistics);

private:
    // General
    const AgentSettings& settings;
    unsigned generation;
    unsigned oldChildCount;
    unsigned childCount;
    unsigned oldMurderCount;
    unsigned murderCount;
    std::string name;

    // Input linear regression
    unsigned actionUpdates;
    std::vector<float> perceptMean;
    std::vector<float> actionsMean;
    std::vector<std::vector<float>> varX;
    std::vector<std::vector<float>> covXY;
    void networkRegression();

    float orientation; // In degrees
    float energy; // Between 0 and maxEnergy
    float actionCooldown;
    float punchTimer;
    Inventory inventory;

     // Rendering
    sf::Sprite sprite;

    sf::IntRect frame;
    unsigned frameIndex;
    float frameTimer;

    // AI
    std::shared_ptr<MapGenes> genes;
    NetworkStatistics networkStatistics;
    std::vector<float> percept;
    std::vector<float> memory;
    std::vector<float> actions;
    std::vector<float> receptors;

    // Agent path
    float pathTimer;
    bool drawPathNextFrame{};
    std::vector<sf::Vertex> path;
    bool alreadyRegularColor{};

    // Vision variables
    std::vector<sf::Vertex> lineOfSight;
    sf::Vertex orientationLine[2];

    static bool loaded;
    static sf::Texture walkingTexture;
    static sf::Texture punchTexture;

    void constructGenome(size_t inputCount, size_t outputCount);

};


#endif //FAMILYISEVERYTHING_AGENT_H
