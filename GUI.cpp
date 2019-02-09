//
// Created by axelw on 2019-01-06.
//

#include <sstream>
#include "GUI.h"
#include "World.h"
#include "Camera.h"

GUI::GUI(Config &config, sf::RenderWindow *window, World *world, Camera *camera)
        : config(config), window(window), originalWindowSize(window->getSize()), view(window->getDefaultView()),
        world(world), camera(camera){
    font.loadFromFile(R"(C:\Windows\Fonts\consola.ttf)");
    sf::Color gray(120, 120, 120);

    simulationInfo.main = sf::Text(std::string("FPS: ###\nTime factor: ##\nPopulation: ###\nAverage generation: ####")+
            std::string("\nLowest generation: ###\nHighest generation: ###\n"), font);
    simulationInfo.main.setCharacterSize(20);
    simulationInfo.main.setStyle(sf::Text::Regular);
    simulationInfo.main.setFillColor(gray);
    simulationInfo.main.setPosition(10, window->getSize().y-simulationInfo.main.getLocalBounds().height-5);

    std::vector<std::pair<std::string, bool*>> debugValues = {
            {"agentSpawning", &world->agentSpawning},
            {"reloadConfig", &config.shouldReload},
            {"showWorldObjectBounds", &config.render.showWorldObjectBounds},
            {"showQuadtree", &config.render.showQuadtree},
            {"showQuadtreeEntities", &config.render.showQuadtreeEntities},
            {"showVision", &config.render.showVision},
            {"showDistribution", &config.render.showDistribution},
            {"renderOnlyAgents", &config.render.renderOnlyAgents},
            {"visualizeGeneration", &config.render.visualizeGeneration},
            {"visualizeAge", &config.render.visualizeAge},
            {"visualizeChildren", &config.render.visualizeChildren},
            {"visualizeMurders", &config.render.visualizeMurders},
            {"visualizeMushrooms", &config.render.visualizeMushrooms},
            {"visualizeColor", &config.render.visualizeColor}

    };

    sf::Rect<int> distributionBounds(450, window->getSize().y-10, 800, 90);
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

    agentInfo.infoText = agentInfo.energyText;
    agentInfo.infoText.setString("Network layers ##\nPerceptron count: ##\nAge: ###\nGeneration: ###\nChildren: ###\nMurders: ##\nMushrooms: ##\n");
    agentInfo.infoText.setPosition(10, 245);



}

void GUI::draw(float deltaTime, float timeFactor) {
    auto cameraView = window->getView();
    window->setView(view);

    simulationInfo.main.setString("FPS: "+std::to_string(int(1.f/deltaTime))
        +"\nTime factor: "+std::to_string(int(timeFactor))
        +"\nPopulation: "+std::to_string(world->getStatistics().populationCount)
        +"\nAverage generation: "+std::to_string(world->getStatistics().averageGeneration)
        +"\nLowest generation: " + std::to_string(world->getStatistics().lowestGeneration)
        +"\nHighest generation: " + std::to_string(world->getStatistics().highestGeneration));
    window->draw(simulationInfo.main);

    unsigned deltaGeneration = world->getStatistics().highestGeneration - world->getStatistics().lowestGeneration;
    unsigned last = 0;
    if (deltaGeneration != 0 && config.render.showDistribution) {
        double delta = (double) (deltaGeneration + 1) / simulationInfo.populationDistribution.size();
        delta = fmax(delta, 1.0);
        for (unsigned i = 0; i < simulationInfo.populationDistribution.size(); i++) {
            unsigned upTo = world->getStatistics().lowestGeneration + floor(delta * (i + 1));
            unsigned val = 0;

            for (unsigned j = last; j < upTo; j++) {
                try {
                    val += world->getStatistics().populationDistribution.at(j) / (upTo - last);
                } catch (const std::out_of_range& e) { /* This is a gap between the generations */ };
            }
            last = upTo;
            simulationInfo.populationDistribution.at(i).setScale(1, (float) val/world->getStatistics().populationCount);
            window->draw(simulationInfo.populationDistribution.at(i));
        }
    }

    if (config.render.showDebug){
        for (auto& t : simulationInfo.debug){
            t.update();
            window->draw(t.text);
        }
    }

    if (selectedAgent){
        window->draw(agentInfo.agentIdentifier);

        window->draw(agentInfo.energyText);
        agentInfo.energyBar.setScale(selectedAgent->getEnergy()/selectedAgent->getSettings().maxEnergy, 1);
        window->draw(agentInfo.energyBackground);
        window->draw(agentInfo.energyBar);

        window->draw(agentInfo.perceptText);
        agentInfo.perceptVector.draw(window, selectedAgent->getPercept());

        window->draw(agentInfo.actionsText);
        agentInfo.actionVector.draw(window, selectedAgent->getActions());

        agentInfo.infoText.setString(
                "Network layers: "+std::to_string(selectedAgent->getNetworkStatistics().layers)
                +"\nPerceptron count: "+std::to_string(selectedAgent->getNetworkStatistics().perceptronCount)
                +"\nAge: " + std::to_string(selectedAgent->getAge())
                +"\nGeneration: "+ std::to_string(selectedAgent->getGeneration())
                +"\nChildren: "+std::to_string(selectedAgent->getChildCount())
                +"\nMurders: "+std::to_string(selectedAgent->getMurderCount())
                +"\nMushrooms: "+std::to_string(selectedAgent->getInventory().mushrooms));
        window->draw(agentInfo.infoText);
    }
    window->setView(cameraView);
}

void GUI::selectAgent(std::shared_ptr<Agent> agent) {
    if (agent == nullptr){
        selectedAgent.reset();
    }
    else {
        selectedAgent = agent;

        agentInfo.agentIdentifier.setFillColor(agent->getColor());
        agentInfo.agentIdentifier.setString(agent->getName());
    }
}

const std::shared_ptr<Agent> &GUI::getSelectedAgent() const {
    return selectedAgent;
}

bool GUI::click(sf::Vector2i pos) {
    pos = sf::Vector2i(((float) pos.x / window->getSize().x) * originalWindowSize.x,
                       ((float) pos.y / window->getSize().y) * originalWindowSize.y);

    if (config.render.showDebug){
        for (auto& t : simulationInfo.debug){
            if (pointInBox(sf::Vector2f(pos.x, pos.y), t.text.getGlobalBounds())){
                t.click();
                if (t.text.getString().substring(0, 9) == "visualize") {
                    for (auto& c : simulationInfo.debug){
                        if (c.text.getString() != t.text.getString() && c.text.getString().substring(0, 9) == "visualize"){
                            c.set(false);
                        }
                    }
                }
                return true;
            }
        }
    }
    if (pointInBox(sf::Vector2f(pos.x, pos.y), agentInfo.agentIdentifier.getGlobalBounds())){
        camera->followAgent(selectedAgent.get());
        return true;
    }
    if (pointInBox(sf::Vector2f(pos.x, pos.y), simulationInfo.main.getGlobalBounds())){
        printf("Click on info\n");
    }

    return false;
}

void GUI::Toggle::click() {
    *value = !(*value);
    update();
}

void GUI::Toggle::set(bool v) {
    *value = v;
    update();
}

void GUI::Toggle::update() {
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
