//
// Created by axelw on 2019-01-06.
//

#include <sstream>
#include "GUI.h"
#include "World.h"

GUI::GUI(sf::RenderWindow *window, World* world) : window(window), view(window->getDefaultView()), world(world) {
    font.loadFromFile(R"(C:\Windows\Fonts\consola.ttf)");
    sf::Color gray(120, 120, 120);

    simulationInfo.main = sf::Text("FPS: ###\nTime factor: ##\nPopulation: ###\nAverage generation: ####\nHighest generation: ####\n", font);
    simulationInfo.main.setCharacterSize(20);
    simulationInfo.main.setStyle(sf::Text::Regular);
    simulationInfo.main.setFillColor(gray);
    simulationInfo.main.setPosition(10, window->getSize().y-simulationInfo.main.getLocalBounds().height-5);

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
        +"\nPopulation: "+std::to_string(world->getAgents().size())
        +"\nAverage generation: "+std::to_string(world->getStatistics().averageGeneration)
        +"\nHighest generation: "+std::to_string(world->getStatistics().highestGeneration));
    window->draw(simulationInfo.main);

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
        agentInfo.agentIdentifier.setString(agent->getName());
    }
}

const std::shared_ptr<Agent> &GUI::getSelectedAgent() const {
    return selectedAgent;
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

