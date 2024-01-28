//
// Created by Axel on 2018-11-23.
//

#ifndef HUNTERGATHERERS_CAMERA_H
#define HUNTERGATHERERS_CAMERA_H


#include <SFML/Graphics.hpp>
#include <cmath>

#include "Config.h"
#include "Agent.h"

class Camera {
public:
    explicit Camera(Config &config, sf::RenderWindow *window, sf::Vector2f worldSize);
    void update(float deltaT);
    void move(sf::Vector2f offset);
    void zoomTo(float mouseWheelDelta, sf::Vector2<int> mousePosition);
    void followAgent(Agent* agent);
    void resizeWindow(sf::Event::SizeEvent size);

    sf::View getView();
    void setView(sf::View view);
private:
    Config& config;
    sf::RenderWindow *window;
    sf::View view;
    sf::Vector2f worldSize;

    bool followingAgent;
    std::weak_ptr<Agent> agentFollow;
};


#endif //HUNTERGATHERERS_CAMERA_H
