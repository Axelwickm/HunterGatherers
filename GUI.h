//
// Created by axelw on 2019-01-06.
//

#ifndef HUNTERGATHERERS_GUI_H
#define HUNTERGATHERERS_GUI_H


#include <SFML/Graphics/RenderWindow.hpp>
#include "Agent.h"
#include "Camera.h"

class GUI {
public:
    explicit GUI(Config &config, sf::RenderWindow *window, World *world, Camera *camera);
    void draw(float deltaTime, float timeFactor);
    void selectAgent(std::shared_ptr<Agent> agent);
    bool click(sf::Vector2i pos);
    bool hover(sf::Vector2i pos);

    const std::shared_ptr<Agent> &getSelectedAgent() const;

private:
    Config& config;
    sf::RenderWindow *window;
    const sf::Vector2i originalWindowSize;
    const World* world;
    Camera* camera;
    const sf::View &view;
    sf::Font font;

    struct Toggle {
        void click();
        void set(bool v);
        void update();
        bool* value;
        sf::Text text;
    };

    struct {
        sf::Text main;
        std::vector<sf::RectangleShape> populationDistribution;
        std::vector<Toggle> debug;
    } simulationInfo;

    struct VectorRenderer {
        std::size_t hover(sf::Vector2i pos);
        void draw(sf::RenderWindow *window, const std::vector<float> &vec, std::size_t selectedIndex = std::numeric_limits<std::size_t>::max());
        void drawCorr(sf::RenderWindow *window, const std::vector<float> &vec, std::size_t selectedIndex = std::numeric_limits<std::size_t>::max());

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

        sf::Text infoText;

        std::vector<std::string> perceptLabels;
        std::vector<std::string> actionLabels;
    } agentInfo;

    enum AgentVectors {
        VECTOR_NONE,
        VECTOR_PERCEPT,
        VECTOR_ACTIONS
    };

    std::pair<AgentVectors, std::size_t> selectedInput;

    struct {
        bool active = false;
        sf::Vector2i pos;
        std::string text;
    } tooltip;

};


#endif //HUNTERGATHERERS_GUI_H
