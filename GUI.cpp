//
// Created by axelw on 2019-01-06.
//

#include <sstream>
#include "GUI.h"
#include "World.h"

GUI::GUI(Config &config, sf::RenderWindow *window, World *world)
        : config(config), window(window), view(window->getDefaultView()), world(world) {
    font.loadFromFile(R"(C:\Windows\Fonts\consola.ttf)");
    sf::Color gray(120, 120, 120);

    simulationInfo.main = sf::Text("FPS: ###\nTime factor: ##\nPopulation: ###\nAverage generation: ####\n", font);
    simulationInfo.main.setCharacterSize(20);
    simulationInfo.main.setStyle(sf::Text::Regular);
    simulationInfo.main.setFillColor(gray);
    simulationInfo.main.setPosition(10, window->getSize().y-simulationInfo.main.getLocalBounds().height-5);

    std::vector<std::pair<std::string, bool*>> debugValues = {
        {"showWorldObjectBounds", &config.render.showWorldObjectBounds},
        {"showQuadtree", &config.render.showQuadtree},
        {"showQuadtreeEntities", &config.render.showQuadtreeEntities},
        {"showVision", &config.render.showVision},
    };


    sf::Rect<int> distributionBounds(450, window->getSize().y-10, 400, 90);
    simulationInfo.populationDistribution.resize(config.render.bins);
    int binWidth = distributionBounds.width / simulationInfo.populationDistribution.size();
    for (std::size_t i = 0; i < simulationInfo.populationDistribution.size(); i++){
        simulationInfo.populationDistribution.at(i).setFillColor(gray);
        simulationInfo.populationDistribution.at(i).setPosition(distributionBounds.left + i*binWidth, distributionBounds.top);
        simulationInfo.populationDistribution.at(i).setSize(sf::Vector2f(binWidth, distributionBounds.height));
        simulationInfo.populationDistribution.at(i).setOrigin(simulationInfo.populationDistribution.at(i).getSize());
    }

    for (std::size_t i = 0; i < debugValues.size(); i++){
        simulationInfo.debug.push_back({
            debugValues.at(i).second,
            sf::Text(debugValues.at(i).first, font)
        });
        simulationInfo.debug.back().text.setCharacterSize(20);
        simulationInfo.debug.back().text.setPosition(window->getSize().x-300, 10+i*25);
        if (*debugValues.at(i).second){
            simulationInfo.debug.back().text.setFillColor(sf::Color::White);
        }
        else {
            simulationInfo.debug.back().text.setFillColor(sf::Color(120, 120, 120));
        }
    }

    // Agent info
    agentInfo.agentIdentifier = sf::Text("", font);
    agentInfo.agentIdentifier.setCharacterSize(40);
    agentInfo.agentIdentifier.setStyle(sf::Text::Bold);
    agentInfo.agentIdentifier.setFillColor(gray);
    agentInfo.agentIdentifier.setPosition(10, 10);

    agentInfo.energyText = sf::Text("Energy:", font);
    agentInfo.energyText.setCharacterSize(20);
    agentInfo.agentIdentifier.setStyle(sf::Text::Regular);
    agentInfo.energyText.setFillColor(gray);
    agentInfo.energyText.setPosition(10, 60);

    agentInfo.energyBackground = sf::RectangleShape(sf::Vector2f(300, 25));
    agentInfo.energyBackground.setPosition(10, 90);
    agentInfo.energyBackground.setFillColor(sf::Color(50, 50, 50));

    agentInfo.energyBar = sf::RectangleShape(sf::Vector2f(300, 25));
    agentInfo.energyBar.setPosition(10, 90);
    agentInfo.energyBar.setFillColor(sf::Color(120, 120, 120));

    agentInfo.perceptText = agentInfo.energyText;
    agentInfo.perceptText.setString("Percept:");
    agentInfo.perceptText.setPosition(10, 125);
    agentInfo.perceptVector.bounds = sf::Rect<float>(10, 155, 300, 25);

    agentInfo.actionsText = agentInfo.energyText;
    agentInfo.actionsText.setString("Actions:");
    agentInfo.actionsText.setPosition(10, 185);
    agentInfo.actionVector.bounds = sf::Rect<float>(10, 215, 300, 25);


}

void GUI::draw(float deltaTime, float timeFactor) {
    auto cameraView = window->getView();
    window->setView(view);

    simulationInfo.main.setString("FPS: "+std::to_string(int(1.f/deltaTime))
        +"\nTime factor: "+std::to_string(int(timeFactor))
        +"\nPopulation: "+std::to_string(world->getStatistics().populationCount)
        +"\nAverage generation: "+std::to_string(world->getStatistics().averageGeneration));
    window->draw(simulationInfo.main);

    unsigned deltaGeneration = world->getStatistics().highestGeneration - world->getStatistics().lowestGeneration;
    unsigned last =  world->getStatistics().lowestGeneration;
    if (deltaGeneration != 0 && config.render.showDistribution) {
        double delta = (double) (deltaGeneration + 1) / simulationInfo.populationDistribution.size();
        delta = fmax(delta, 1.0);
        for (unsigned i = 0; i < simulationInfo.populationDistribution.size(); i++) {
            unsigned upTo = floor(delta * (i + 1));
            double val = 0;
            if (world->getStatistics().populationDistribution.size() <= upTo){
                break;
            }
            for (unsigned j = last; j < upTo; j++) {
                val += world->getStatistics().populationDistribution.at(j) / (upTo - last);
            }
            last = upTo;
            simulationInfo.populationDistribution.at(i).setScale(1, val/world->getStatistics().populationCount);
            window->draw(simulationInfo.populationDistribution.at(i));
        }
    }

    if (config.render.showDebug){
        for (auto& t : simulationInfo.debug){
            window->draw(t.text);
        }
    }

    if (selectedAgent){
        window->draw(agentInfo.agentIdentifier);

        window->draw(agentInfo.energyText);
        agentInfo.energyBar.setScale(selectedAgent->getEnergy()/selectedAgent->getMaxEnergy(), 1);
        window->draw(agentInfo.energyBackground);
        window->draw(agentInfo.energyBar);

        window->draw(agentInfo.perceptText);
        agentInfo.perceptVector.draw(window, selectedAgent->getPercept());

        window->draw(agentInfo.actionsText);
        agentInfo.actionVector.draw(window, selectedAgent->getActions());
    }
    window->setView(cameraView);
}

void GUI::selectAgent(std::shared_ptr<Agent> agent) {
    if (agent == nullptr){
        selectedAgent.reset();
    }
    else {
        selectedAgent = agent;

        /*std::ostringstream address;
        address << (void const *) agent.get();*/
        agentInfo.agentIdentifier.setFillColor(agent->getColor());
        agentInfo.agentIdentifier.setString(agent->getName());
    }
}

const std::shared_ptr<Agent> &GUI::getSelectedAgent() const {
    return selectedAgent;
}

bool GUI::click(sf::Vector2i pos) {
    if (config.render.showDebug){
        for (auto& t : simulationInfo.debug){
            if (pointInBox(sf::Vector2f(pos.x, pos.y), t.text.getGlobalBounds())){
                t.click();
                return true;
            }
        }
    }
    if (pointInBox(sf::Vector2f(pos.x, pos.y), simulationInfo.main.getGlobalBounds())){
        printf("Click on info\n");
    }

    return false;
}

void GUI::Toggle::click() {
    *value = !(*value);
    if (*value){
        text.setFillColor(sf::Color::White);
    }
    else {
        text.setFillColor(sf::Color(120, 120, 120));
    }
}

void GUI::VectorRenderer::draw(sf::RenderWindow *window, const std::vector<float> &vec) {
    if (vec.size() != rectangles.size()){
        rectangles.clear();
        const auto c = vec.size();
        rectangles.reserve(c);
        for (std::size_t i = 0; i < c; i++){
            rectangles.emplace_back(sf::Vector2f(bounds.width/c, bounds.height));
            rectangles.back().setPosition(bounds.left + i*bounds.width/c, bounds.top);
            rectangles.back().setOutlineThickness(1);
            rectangles.back().setOutlineColor(sf::Color(50, 50, 50));
        }
    }
    for (std::size_t i = 0; i < vec.size(); i++ ){
        const float a = vec.at(i);
        rectangles.at(i).setFillColor(sf::Color(a*200, a*200, a*200));
        window->draw(rectangles.at(i));
    }
}
