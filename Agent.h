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

    const sf::Color &getColor() const;

    const std::string &getName() const;

    static void loadResources();

    MapGenes *getGenes() const;

    unsigned int getGeneration() const;
    void setGeneration(unsigned int generation);

    void update(float deltaTime) override;
    void draw(sf::RenderWindow *window, float deltaTime) override;

    void updatePercept(float deltaTime);

    const std::vector<float> &getPercept() const;
    void setPercept(const std::vector<float> &percept);

    const std::vector<float> &getActions() const;
    void setActions(const std::vector<float> &action);

    float getOrientation() const;
    void setOrientation(float orientation);


    float getEnergy() const;
    void setEnergy(float energy);

    float getMaxEnergy() const;
    void setMaxEnergy(float maxEnergy);

private:
    // General
    unsigned generation;
    std::string name;
    float orientation; // In degrees
    float maxEnergy;
    float energy; // Between 0 and maxEnergy
    float energyLossRate;
    float maxSpeed;
    float turnFactor;

     // Rendering
    sf::Sprite sprite;
    sf::IntRect frame;
    unsigned frameIndex;
    float frameTimer;
    sf::Color color;

    // AI
    std::shared_ptr<MapGenes> genes;
    std::vector<float> percept;
    std::vector<float> actions;
    std::vector<float> receptors;


    // Vision variables,
    unsigned receptorCount;
    float visibilityDistance;
    float visualReactivity;
    float FOV;
    std::vector<sf::Vertex> lineOfSight;
    sf::Vertex orientationLine[2];

    static bool loaded;
    static sf::Texture walkingTexture;

    void constructGenome(size_t inputCount, size_t outputCount);
};


#endif //FAMILYISEVERYTHING_AGENT_H
