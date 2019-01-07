//
// Created by axelw on 2019-01-06.
//

#include <sstream>
#include "GUI.h"

GUI::GUI(sf::RenderWindow* window) : window(window), view(window->getDefaultView()){
    font.loadFromFile(R"(C:\Windows\Fonts\consola.ttf)");
    sf::Color gray(120, 120, 120);

    // TODO: monitor size scaling
    // Agent info
    agentInfo.agentIdentifier = sf::Text("AgentID", font);
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

void GUI::draw(float deltaTime) {
    auto cameraView = window->getView();
    window->setView(view);

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

        std::ostringstream address;
        address << (void const *) agent.get();
        agentInfo.agentIdentifier.setString(address.str());
    }
}

const std::shared_ptr<Agent> &GUI::getSelectedAgent() const {
    return selectedAgent;
}

void GUI::VectorRenderer::draw(sf::RenderWindow* window, const std::vector<float> &vec) {
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

