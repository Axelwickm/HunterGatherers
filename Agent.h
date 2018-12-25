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
    Agent(World* world, sf::Vector2f position);
    void loadResources();
    const MapGenes &getGenes() const;

    void update(float deltaTime) override;
    void draw(sf::RenderWindow *window, float deltaTime) override;

    void updatePercept(float deltaTime);

    const std::vector<float> &getPercept() const;
    void setPercept(const std::vector<float> &percept);

    const std::vector<float> &getActions() const;
    void setActions(const std::vector<float> &action);

    float getOrientation() const;
    void setOrientation(float orientation);

private:
    // General
    sf::Sprite r;
    sf::IntRect frame;
    unsigned frameIndex;
    sf::Clock frameTimer;
    float orientation; // In degrees

    // AI
    MapGenes genes;
    std::vector<float> percept;
    std::vector<float> actions;

    // Vision variables
    float visibility;
    float visualReactivity;
    float FOV;
    const unsigned static acuity = 4;
    sf::Vertex lineOfVision[acuity*2];

    std::vector<float> receptors;

    static bool loaded;
    static sf::Texture walkingTexture;

};


#endif //FAMILYISEVERYTHING_AGENT_H
