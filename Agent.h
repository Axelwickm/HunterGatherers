//
// Created by Axel on 2018-11-21.
//

#ifndef FAMILYISEVERYTHING_AGENT_H
#define FAMILYISEVERYTHING_AGENT_H

#include "WorldObject.h"
#include "Gene.h"

#include <SFML/Graphics.hpp>
#include <vector>

class Agent : public WorldObject {
public:
    Agent(World *world, sf::Vector2f position, float orientation);
    Agent(const Agent& other);

    static void loadResources();
    MapGenes* getGenes() const;

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

private:
    // General
    float orientation; // In degrees
    float energy; // Between 0 and 100

     // Rendering
    sf::Sprite sprite;
    sf::IntRect frame;
    unsigned frameIndex;
    float frameTimer;

    // AI
    std::shared_ptr<MapGenes> genes;
    std::vector<float> percept;
    std::vector<float> actions;
    std::vector<float> receptors;


    // Vision variables,
    float visibility;
    float visualReactivity;
    float FOV;
    const unsigned static acuity = 4;
    sf::Vertex lineOfVision[acuity*2];
    sf::Vertex orientationLine[2];

    static bool loaded;
    static sf::Texture walkingTexture;

};


#endif //FAMILYISEVERYTHING_AGENT_H
