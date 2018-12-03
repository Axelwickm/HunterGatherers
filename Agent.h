//
// Created by Axel on 2018-11-21.
//

#ifndef FAMILYISEVERYTHING_AGENT_H
#define FAMILYISEVERYTHING_AGENT_H

#include "WorldObject.h"

#include <SFML/Graphics.hpp>
#include <vector>

struct NeuralNet {
    unsigned inputBandwidth;
    unsigned outputBandwidth;

    unsigned maxLayerSize;
    unsigned layerCount;
    std::vector<unsigned> layerSizes;
    std::vector<float> layerBiases;
    std::vector<float> layerWeights;
};

class Agent : public WorldObject {
public:
    Agent(World* world, sf::Vector2f position);
    const NeuralNet &getNeuralNet() const;

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
    NeuralNet neuralNet = {
            4, 3,
            0, 3,
            {4, 2, 2, 3},                            // Include sense and output bandwidth
            {0.3, 0.4, 0.6},
            {0.5, 0.7, 0.9, 0.8, 0.4, 0.8, 0.1, 0.0, // 2*senseBandwidth (4)
             0.5, 0.6, 0.5, 0.7,                     // 2*2
             0.1, 0.5, 0.0, 0.0, 0.7, 0.63},        // Output bandwidth (3) * prev (2)
    };

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
