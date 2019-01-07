//
// Created by axelw on 2019-01-06.
//

#ifndef HUNTERGATHERERS_GUI_H
#define HUNTERGATHERERS_GUI_H


#include <SFML/Graphics/RenderWindow.hpp>
#include "Agent.h"

class GUI {
public:
    explicit GUI(sf::RenderWindow* window);
    void draw(float deltaTime);
    void selectAgent(std::shared_ptr<Agent> agent);

    const std::shared_ptr<Agent> &getSelectedAgent() const;

private:
    sf::RenderWindow* window;
    const sf::View& view;
    sf::Font font;

    struct VectorRenderer {
        void draw(sf::RenderWindow* window, const std::vector<float> &vec);

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
