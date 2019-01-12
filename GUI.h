//
// Created by axelw on 2019-01-06.
//

#ifndef HUNTERGATHERERS_GUI_H
#define HUNTERGATHERERS_GUI_H


#include <SFML/Graphics/RenderWindow.hpp>
#include "Agent.h"

class GUI {
public:
    explicit GUI(Config &config, sf::RenderWindow *window, World *world);
    void draw(float deltaTime, float timeFactor);
    void selectAgent(std::shared_ptr<Agent> agent);
    bool click(sf::Vector2i pos);

    const std::shared_ptr<Agent> &getSelectedAgent() const;

private:
    Config& config;
    sf::RenderWindow *window;
    const World* world;
    const sf::View &view;
    sf::Font font;

    struct Toggle {
        void click();
        bool* value;
        sf::Text text;
    };

    struct {
        sf::Text main;
        std::vector<sf::RectangleShape> populationDistribution;
        std::vector<Toggle> debug;
    } simulationInfo;

    struct VectorRenderer {
        void draw(sf::RenderWindow *window, const std::vector<float> &vec);

        sf::Rect<float> bounds;
        std::vector<sf::RectangleShape> rectangles;
    };

    std::shared_ptr<Agent> selectedAgent;

    struct {
        sf::Text agentIdentifier;
        sf::Text energyText;
        sf::RectangleShape energyBackground;
        sf::RectangleShape energyBar;

        sf::Text perceptText;
        VectorRenderer perceptVector;
        sf::Text actionsText;
        VectorRenderer actionVector;
    } agentInfo;




};


#endif //HUNTERGATHERERS_GUI_H
