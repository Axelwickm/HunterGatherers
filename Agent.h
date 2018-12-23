//
// Created by Axel on 2018-11-21.
//

#ifndef FAMILYISEVERYTHING_AGENT_H
#define FAMILYISEVERYTHING_AGENT_H

#include "WorldObject.h"
#include "Gene.h"

#include <SFML/Graphics.hpp>
#include <vector>

/*
struct NeuralNet {
    unsigned inputBandwidth;
    unsigned outputBandwidth;

    unsigned maxLayerSize;
    unsigned layerCount;
    std::vector<unsigned> layerSizes;
    std::vector<float> layerBiases;
    std::vector<float> layerWeights;
};*/

class Agent : public WorldObject {
public:
    Agent(World* world, sf::Vector2f position);
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
    sf::RectangleShape r;
    float orientation; // In degrees

    // AI
    MapGenes genes;
    std::vector<float> percept;
    std::vector<float> actions;

    // Visual variables
    float visibility;
    float visualReactivity;
    float FOV;
    const unsigned static acuity = 4;
    sf::Vertex lineOfVision[acuity*2];

    std::vector<float> receptors;


};


#endif //FAMILYISEVERYTHING_AGENT_H
