//
// Created by axelw on 2019-01-06.
//

#ifndef HUNTERGATHERERS_GUI_H
#define HUNTERGATHERERS_GUI_H


#include <SFML/Graphics/RenderWindow.hpp>
#include "Agent.h"
#include "Camera.h"
#include "utils.h"
#include "World.h"

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
        Toggle(const std::string& name, bool *value, std::vector<Toggle> subtoggles = std::vector<Toggle>(),
                sf::Color color = sf::Color(200, 200, 200));
        Toggle(const std::string& name, bool *value, Toggle* parent,
                sf::Color color = sf::Color(200, 200, 200));
        void click();
        void set(bool v);
        void update();
        sf::Text text;
        bool* value;
        sf::Color color;
        std::vector<Toggle> subToggles;
        bool exclusiveSubs = false;
        Toggle* parent;
        bool hovered;
    };

    struct SimulationInfo {
        sf::Text main;
        std::vector<sf::RectangleShape> populationDistribution;
        std::vector<Toggle> debug;
    } simulationInfo;

    struct LineGraph {
        void update(const World *world);
        void draw(sf::RenderWindow *window, sf::Vector2f orgSize);

        std::string name;
        sf::Color color;
        bool* shouldRender;
        unsigned yPixelOffset = 0;
        sf::Text valueText;
        unsigned maxPoints = 20;
        sf::VertexArray verts;
        sf::Vector2f min = {std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
        sf::Vector2f max = {std::numeric_limits<float>::min(), std::numeric_limits<float>::min()};
        unsigned lastUpdateFrame = 0;
    };


    std::vector<LineGraph> lineGraphs;

    struct Spectrogram {
        void update(const World *world);
        void draw(sf::RenderWindow *window, const sf::Vector2f orgSize);

        std::string name;
        bool* shouldRender;
        float stride = 1;
        unsigned markerWidth = 5;
        sf::Vector2u currentSize;
        unsigned startHeight = 20;
        std::size_t downsamplingTriggerH = 400;
        std::size_t downsamplingTriggerW = 2000;

        std::vector<std::vector<WorldStatistics::ColorValue>> newValues;
        unsigned perRow = 1;
        unsigned perColumn = 1; unsigned columnCounter = 0;
        std::vector<sf::Color> colorColumn;
        std::vector<unsigned> colorColumnCount;

        Contiguous2dVector<sf::Color> spectrogram;
        float minVal = std::numeric_limits<float>::max();
        float maxVal = std::numeric_limits<float>::min();
        unsigned lastUpdateFrame = 0;
    };

    std::vector<Spectrogram> spectrograms;

    struct VectorRenderer {
        std::size_t hover(sf::Vector2i pos);
        void draw(sf::RenderWindow *window, const std::vector<float> &vec, std::size_t selectedIndex = std::numeric_limits<std::size_t>::max());
        void drawCorr(sf::RenderWindow *window, const std::vector<float> &vec, std::size_t selectedIndex = std::numeric_limits<std::size_t>::max());

        sf::Rect<float> bounds;
        std::vector<sf::RectangleShape> rectangles;
    };

    std::shared_ptr<Agent> selectedAgent;

    struct agentInfo {
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

    struct Tooltip {
        bool active = false;
        sf::Vector2i pos;
        std::string text;
    } tooltip;

};


#endif //HUNTERGATHERERS_GUI_H
