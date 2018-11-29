//
// Created by Axel on 2018-11-21.
//

#include <iostream>
#include <cmath>

#include "Agent.h"
#include "Config.h"
#include "utils.cpp"
#include "World.h"

#define PI 3.14159265f

Agent::Agent(World* world, sf::Vector2f position) : WorldObject(world, position) {
    orientation = 75;
    setAccelerationFactor(0.25);

    r.setSize(sf::Vector2f(10, 10));
    r.setOrigin(5, 5);
    r.setFillColor(sf::Color::Red);

    visibility = 200;
    FOV = 110;
    visualReactivity = 3;

    receptors.resize(acuity);
    std::fill(std::begin(receptors), std::end(receptors), 0.f);

    lineOfVision[0] = sf::Vertex(sf::Vector2f(0,0));
    lineOfVision[1] = sf::Vector2f(0,0);
    lineOfVision[0].color = sf::Color::Cyan;
    lineOfVision[1].color = sf::Color::Cyan;

    neuralNet.maxLayerSize = std::max(*std::max_element(std::begin(neuralNet.layerSizes), std::end(neuralNet.layerSizes)),
            neuralNet.inputBandwidth);

    percept = std::vector<float>(neuralNet.maxLayerSize);
    std::fill(std::begin(percept), std::end(percept), 0.f);
}

void Agent::update(float deltaTime) {
    WorldObject::update(deltaTime);
}

void Agent::draw(sf::RenderWindow *window, float deltaTime) {
    r.setPosition(getPosition());
    window->draw(r);

    if (RenderSettings::showVision){
        window->draw(lineOfVision, 2*receptors.size(), sf::Lines);
    }
}

void Agent::updatePercept(float deltaTime) {

    // Calculate perceptors

    sf::Vector2f visionEnd = getPosition() + sf::Vector2f(visibility, 0);
    sf::Vector2f dV = visionEnd - getPosition();

    for (size_t i = 0; i < receptors.size(); i++){
        float angle = ((float) orientation - FOV/2.f + FOV*((float) i/receptors.size()))*PI/180.f;
        sf::Vector2f lineEnd = {
                dV.x * cosf(angle) - dV.y * sinf(angle),
                dV.x * sinf(angle) - dV.y * cosf(angle)
        };

        std::vector<std::shared_ptr<WorldObject> > nl;
        quadtree->searchNearLine(nl, getPosition(), getPosition()+lineEnd);

        for (auto &n : nl){
            if (n.get() != this){
                sf::Vector2f a = n->getPosition();
                sf::Vector2f b = n->getPosition() + sf::Vector2f(10, 10);
                if (lineIntersectWithBox(getPosition(), getPosition()+lineEnd, a, b)){
                    sf::Vector2f dPos = n->getPosition() - getPosition();
                    float change = deltaTime*visualReactivity*(1.f-(dPos.x*dPos.x+dPos.y*dPos.y)/(visibility*visibility));
                    receptors[i] = (1-deltaTime*change)*receptors[i] + change;
                }
            }
        }

        receptors[i] = (1-deltaTime*visualReactivity)*receptors[i] + 0;
        percept.at(i) = receptors[i];

        if (RenderSettings::showVision) {
            lineOfVision[i*2].position = getPosition();
            lineOfVision[i*2+1].position = getPosition() + lineEnd;
            lineOfVision[i*2].color =   sf::Color(245*receptors[i]+10, 245*receptors[i]+10, 245*receptors[i]+10, 255);
            lineOfVision[i*2+1].color = sf::Color(245*receptors[i]+10, 245*receptors[i]+10, 245*receptors[i]+10, 255);
        }

    }
}

const NeuralNet &Agent::getNeuralNet() const {
    return neuralNet;
}

double Agent::getOrientation() const {
    return orientation;
}

void Agent::setOrientation(double orientation) {
    Agent::orientation = orientation;
}

