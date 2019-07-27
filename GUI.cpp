//
// Created by axelw on 2019-01-06.
//


#include <sstream>
#include <utility>

#include "GUI.h"
#include "World.h"
#include "Camera.h"

GUI::GUI(Config &config, sf::RenderWindow *window, World *world, Camera *camera)
        : config(config), window(window), originalWindowSize(window->getSize()), view(window->getDefaultView()),
        world(world), camera(camera) {
    font.loadFromFile(R"(C:\Windows\Fonts\consola.ttf)");
    sf::Color gray(120, 120, 120);

    simulationInfo.main = sf::Text(std::string("FPS: ###\nTime factor: ##\nPopulation: ###\nAverage generation: ####")+
            std::string("\nLowest generation: ###\nHighest generation: ###\n"), font);
    simulationInfo.main.setCharacterSize(20);
    simulationInfo.main.setStyle(sf::Text::Regular);
    simulationInfo.main.setFillColor(gray);
    simulationInfo.main.setPosition(10, window->getSize().y-simulationInfo.main.getLocalBounds().height-5);


    simulationInfo.debug = {
    /* 0 */ Toggle("agentSpawning", &world->agentSpawning),
    /* 1 */ Toggle("reloadConfig", &config.shouldReload),
    /* 2 */ Toggle("showWorldObjectBounds", &config.render.showWorldObjectBounds),
    /* 3 */ Toggle("showQuadtree", &config.render.showQuadtree),
    /* 4 */ Toggle("showQuadtreeEntities", &config.render.showQuadtreeEntities),
    /* 5 */ Toggle("showVision", &config.render.showVision),
    /* 6 */ Toggle("showLineGraph", &config.render.showLineGraph),
    /* 7 */ Toggle("showDistribution", &config.render.showDistribution),
    /* 8 */ Toggle("renderOnlyAgents", &config.render.renderOnlyAgents),
    /* 9 */ Toggle("visualizeGeneration", &config.render.visualizeGeneration),
    /* 10*/ Toggle("visualizeAge", &config.render.visualizeAge),
    /* 11*/ Toggle("visualizeChildren", &config.render.visualizeChildren),
    /* 12*/ Toggle("visualizeMurders", &config.render.visualizeMurders),
    /* 13*/ Toggle("visualizeMushrooms", &config.render.visualizeMushrooms),
    /* 14*/ Toggle("visualizeColor", &config.render.visualizeColor)
    };

    const std::vector<std::array<int, 3>> subColors = {
        {133, 92, 117},
        {217, 175, 107},
        {175, 100, 88},
        {115, 111, 76},
        {82, 106, 131},
        {98, 83, 119},
        {104, 133, 92},
        {156, 156, 94},
        {160, 97, 119},
        {140, 120, 93},
        {70, 115, 120},
        {124, 124, 124}
    };

    for (unsigned  i = 0; i < simulationInfo.debug.size(); i++){
        auto &toggle = simulationInfo.debug.at(i);
        toggle.text.setFont(font);
        toggle.text.setPosition(window->getSize().x-300, 10+i*25);

        if (toggle.text.getString().toAnsiString() == "showLineGraph"){
            toggle.subToggles = {
                    Toggle("population", &config.render.graphPopulation, &toggle),
                    Toggle("mean gen.", &config.render.graphAverageGeneration, &toggle),
                    Toggle("births", &config.render.graphBirths, &toggle),
                    Toggle("murders", &config.render.graphMurders, &toggle)
            };

            for (std::size_t j = 0; j < toggle.subToggles.size(); j++){
                toggle.subToggles.at(j).text.setPosition(
                        window->getSize().x-400,
                        toggle.text.getPosition().y+j*12
                );
                toggle.subToggles.at(j).text.setCharacterSize(15);
                toggle.subToggles.at(j).text.setFont(font);
                auto col = subColors.at(j);
                toggle.subToggles.at(j).color = sf::Color(col[0], col[1], col[2]);
                toggle.subToggles.at(j).update();
                lineGraphs.push_back((LineGraph) {
                    .name = toggle.subToggles.at(j).text.getString().toAnsiString(),
                    .color = sf::Color(col[0], col[1], col[2]),
                    .shouldRender = toggle.subToggles.at(j).value,
                    .yPixelOffset = (unsigned) j
                });
            }
        }
    }

    sf::Rect<int> distributionBounds(450, window->getSize().y-10, 800, 90);
    simulationInfo.populationDistribution.resize(config.render.bins);
    int binWidth = distributionBounds.width / simulationInfo.populationDistribution.size();
    for (std::size_t i = 0; i < simulationInfo.populationDistribution.size(); i++){
        simulationInfo.populationDistribution.at(i).setFillColor(gray);
        simulationInfo.populationDistribution.at(i).setPosition(distributionBounds.left + i*binWidth, distributionBounds.top);
        simulationInfo.populationDistribution.at(i).setSize(sf::Vector2f(binWidth, distributionBounds.height));
        simulationInfo.populationDistribution.at(i).setOrigin(simulationInfo.populationDistribution.at(i).getSize());
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

    // Correlation stuff
    selectedInput = {VECTOR_NONE, 0};

}

void GUI::draw(float deltaTime, float timeFactor) {
    // UI camera-view
    auto cameraView = window->getView();
    window->setView(view);

    { // Draw current world statistics
        simulationInfo.main.setString("FPS: " + std::to_string(int(1.f / deltaTime))
                                      + "\nTime factor: " + std::to_string(int(timeFactor))
                                      + "\nPopulation: " + std::to_string(world->getStatistics().populationCount)
                                      + "\nAverage generation: " +
                                      std::to_string(world->getStatistics().averageGeneration)
                                      + "\nLowest generation: " +
                                      std::to_string(world->getStatistics().lowestGeneration)
                                      + "\nHighest generation: " +
                                      std::to_string(world->getStatistics().highestGeneration));

        window->draw(simulationInfo.main);
    }

    if (config.render.showLineGraph){
        for (auto &lg : lineGraphs){
            if (!*lg.shouldRender) continue;
            lg.draw(window, world, originalWindowSize);
        }
    }

    { // Draw distribution
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
                    } catch (const std::out_of_range &e) { /* This is a gap between the generations */ };
                }
                last = upTo;
                simulationInfo.populationDistribution.at(i).setScale(1, (float) val /
                                                                        world->getStatistics().populationCount);
                window->draw(simulationInfo.populationDistribution.at(i));
            }
        }
    }

    // Draw debug info
    if (config.render.showDebug){
        for (auto& t : simulationInfo.debug){
            t.update();
            window->draw(t.text);

            if (t.hovered){
                for (auto &sub : t.subToggles){
                    window->draw(sub.text);
                }
            }
        }
    }

    { // Draw information about the selected agent (if there is one)
        if (selectedAgent) {
            window->draw(agentInfo.agentIdentifier);

            window->draw(agentInfo.energyText);
            agentInfo.energyBar.setScale(selectedAgent->getEnergy() / selectedAgent->getSettings().maxEnergy, 1);
            window->draw(agentInfo.energyBackground);
            window->draw(agentInfo.energyBar);

            if (selectedInput.first != VECTOR_NONE) {

                if (selectedInput.first == VECTOR_PERCEPT) {
                    auto correlation = selectedAgent->getRegressionActions(selectedInput.second);

                    window->draw(agentInfo.perceptText);
                    agentInfo.perceptVector.draw(window, selectedAgent->getPercept(), selectedInput.second);

                    window->draw(agentInfo.actionsText);
                    agentInfo.actionVector.drawCorr(window, correlation);
                } else {
                    auto correlation = selectedAgent->getRegressionPercept(selectedInput.second);

                    window->draw(agentInfo.perceptText);
                    agentInfo.perceptVector.drawCorr(window, correlation);

                    window->draw(agentInfo.actionsText);
                    agentInfo.actionVector.draw(window, selectedAgent->getActions(), selectedInput.second);

                }

                // http://ci.columbia.edu/ci/premba_test/c0331/s7/s7_5.html
            } else {
                window->draw(agentInfo.perceptText);
                agentInfo.perceptVector.draw(window, selectedAgent->getPercept());

                window->draw(agentInfo.actionsText);
                agentInfo.actionVector.draw(window, selectedAgent->getActions());
            }

            agentInfo.infoText.setString(
                    "Network layers: " + std::to_string(selectedAgent->getNetworkStatistics().layers)
                    + "\nPerceptron count: " + std::to_string(selectedAgent->getNetworkStatistics().perceptronCount)
                    + "\nAge: " + std::to_string(selectedAgent->getAge())
                    + "\nGeneration: " + std::to_string(selectedAgent->getGeneration())
                    + "\nChildren: " + std::to_string(selectedAgent->getChildCount())
                    + "\nMurders: " + std::to_string(selectedAgent->getMurderCount())
                    + "\nMushrooms: " + std::to_string(selectedAgent->getInventory().mushrooms));

            window->draw(agentInfo.infoText);
        }
    }

    // Draw tooltip
    if (tooltip.active){
        sf::Text text(tooltip.text, font, 10);
        text.setPosition((sf::Vector2f) tooltip.pos + sf::Vector2f(0, 20));
        window->draw(text);
    }

    // Reset camera view
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

        auto settings = agent->getSettings();

        // Apply percept labels
        agentInfo.perceptLabels = std::vector<std::string>();
        if (settings.perceiveCollision) agentInfo.perceptLabels.emplace_back("colliding");
        for (std::size_t i = 0; i < agent->getReceptors().size(); i++)
            agentInfo.perceptLabels.emplace_back("receptor " + std::to_string(i));
        if (settings.perceiveColor){
            agentInfo.perceptLabels.emplace_back("receptors red");
            agentInfo.perceptLabels.emplace_back("receptors green");
            agentInfo.perceptLabels.emplace_back("receptors blue");
        }
        if (settings.perceiveEnergyLevel) agentInfo.perceptLabels.emplace_back("energy");
        if (settings.perceiveMushroomCount) agentInfo.perceptLabels.emplace_back("mushrooms");
        for (std::size_t i = 0; i < agent->getMemory().size(); i++)
            agentInfo.perceptLabels.emplace_back("memory "+std::to_string(i));

        // Apply action labels
        agentInfo.actionLabels = std::vector<std::string>();
        if (settings.canWalk) agentInfo.actionLabels.emplace_back("walk");
        if (settings.canTurn) agentInfo.actionLabels.emplace_back("turn right");
        if (settings.canTurn) agentInfo.actionLabels.emplace_back("turn left");
        if (settings.canReproduce) agentInfo.actionLabels.emplace_back("reproduce");
        if (settings.canEat) agentInfo.actionLabels.emplace_back("eat");
        if (settings.canPlace) agentInfo.actionLabels.emplace_back("place mushroom");
        if (settings.canPunch) agentInfo.actionLabels.emplace_back("punch");
        for (std::size_t i = 0; i < agent->getMemory().size(); i++)
            agentInfo.actionLabels.emplace_back("memory "+std::to_string(i));

    }

    selectedInput = {VECTOR_NONE, 0};
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
            for (auto &sub : t.subToggles){
                if (pointInBox(sf::Vector2f(pos.x, pos.y), sub.text.getGlobalBounds())){
                    sub.click();
                    return true;
                }
            }
        }
    }

    if (selectedAgent){
        if (pointInBox(sf::Vector2f(pos.x, pos.y), agentInfo.agentIdentifier.getGlobalBounds())){
            camera->followAgent(selectedAgent.get());
            return true;
        }

        if (pointInBox(sf::Vector2f(pos.x, pos.y), simulationInfo.main.getGlobalBounds())){
            printf("Click on info\n");
        }

        std::size_t perceptVectorPos = agentInfo.perceptVector.hover(pos);
        if (perceptVectorPos != std::numeric_limits<std::size_t>::max()){
            selectedInput = {VECTOR_PERCEPT, perceptVectorPos};
            return true;
        }

        std::size_t actionVectorPos = agentInfo.actionVector.hover(pos);
        if (actionVectorPos != std::numeric_limits<std::size_t>::max()){
            selectedInput = {VECTOR_ACTIONS, actionVectorPos};
            return true;
        }
    }

    return false;
}

bool GUI::hover(sf::Vector2i pos) {
    pos = sf::Vector2i(((float) pos.x / window->getSize().x) * originalWindowSize.x,
                       ((float) pos.y / window->getSize().y) * originalWindowSize.y);

    if (config.render.showDebug){
        std::function<bool(Toggle&)> hoverToggle = [&](Toggle &t) -> bool {
            bool anyHovered = false;
            if (t.hovered){
                for (auto &sub : t.subToggles){
                    anyHovered |= hoverToggle(sub);
                }
            }
            sf::FloatRect bounds = t.text.getGlobalBounds();
            if (t.parent != nullptr){
                bounds.width += 80;
                bounds.height += 80;
            }
            if (anyHovered || pointInBox(sf::Vector2f(pos.x, pos.y), bounds)){
                t.hovered = true;
                return true;
            }
            t.hovered = false;
            return false;
        };

        for (auto &toggle : simulationInfo.debug){
            hoverToggle(toggle);
        }
    }

    if (selectedAgent){
        std::size_t perceptVectorPos = agentInfo.perceptVector.hover(pos);
        if (perceptVectorPos != std::numeric_limits<std::size_t>::max()){
            tooltip.active = true;
            tooltip.pos = pos;
            tooltip.text = agentInfo.perceptLabels.at(perceptVectorPos);
            return true;
        }

        std::size_t actionVectorPos = agentInfo.actionVector.hover(pos);
        if (actionVectorPos!= std::numeric_limits<std::size_t>::max()){
            tooltip.active = true;
            tooltip.pos = pos;
            tooltip.text = agentInfo.actionLabels.at(actionVectorPos);
            return true;
        }
    }

    tooltip.active = false;
    return false;
}

void GUI::LineGraph::draw(sf::RenderWindow *window, const World *world, const sf::Vector2i orgSize) {
    // Set all the vertex data
    verts.clear();
    auto &stats = world->getHistoricalStatistics();
    verts.resize(stats.size());
    verts.setPrimitiveType(sf::LineStrip);

    if (stats.empty())
        return;

    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::min();

    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::min();

    for (std::size_t i = 0; i < stats.size(); i++){
        const auto &s = stats[i];
        float x, y;
        x = s.timestamp;
        if (name == "population")
            y = s.populationCount;
        else if (name == "mean gen.")
            y = s.averageGeneration;
        else if (name == "births")
            y = s.births;
        else if (name == "murders")
            y = s.murders;
        else
            throw std::runtime_error("Line Graph datum "+name+" doesn't exist");

        verts[i] = sf::Vector2f(x, y);

        minX = std::min(minX, x);
        maxX = std::max(maxX, x);

        minY = std::min(minY, y);
        maxY = std::max(maxY, y);
    }

    for (std::size_t i = 0; i < stats.size(); i++){
        float x = verts[i].position.x;
        float y = verts[i].position.y;
        //  Normalize
        x = (x - minX) / (maxX - minX);
        y = 1.f - (y - minY) / (maxY - minY);

        // Place on screen
        auto wSize = sf::Vector2f(orgSize);
        x = wSize.x*0.15f + x*wSize.x*0.80;
        y = wSize.y - 200 + y*120 + yPixelOffset;

        verts[i] = sf::Vertex({x, y}, color);
    }

    // Draw graph
    window->draw(verts);
}


GUI::Toggle::Toggle(std::string name, bool *value, std::vector<Toggle> subToggles, sf::Color color) :
color(color){
    text.setString(name);
    text.setCharacterSize(20);

    Toggle::value = value;
    Toggle::subToggles = std::move(subToggles);
    Toggle::hovered = false;
    Toggle::parent = nullptr;

    update();
}

GUI::Toggle::Toggle(std::string name, bool *value, Toggle* parent, sf::Color color) :
Toggle(name, value, std::vector<Toggle>(), color) {
    Toggle::parent = parent;
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
        text.setFillColor(color);
    }
    else {
        text.setFillColor(sf::Color(80, 80, 80));
        for (auto &sub : subToggles){
            sub.hovered = false;
        }
    }
}

std::size_t GUI::VectorRenderer::hover(sf::Vector2i pos) {
    for (std::size_t i = 0; i < rectangles.size(); i++){
        if (pointInBox(sf::Vector2f(pos.x, pos.y), rectangles.at(i).getGlobalBounds())){
            return i;
        }
    }
    return std::numeric_limits<std::size_t>::max();
}

void GUI::VectorRenderer::draw(sf::RenderWindow *window, const std::vector<float> &vec, std::size_t selectedIndex) {
    if (vec.size() != rectangles.size()){
        rectangles.clear();
        const auto c = vec.size();
        rectangles.reserve(c);
        for (std::size_t i = 0; i < c; i++){
            rectangles.emplace_back(sf::Vector2f(bounds.width/c, bounds.height));
            rectangles.back().setPosition(bounds.left + i*(bounds.width/c+2.f), bounds.top);
            rectangles.back().setOutlineThickness(1);
            rectangles.back().setOutlineColor(sf::Color(50, 50, 50));
        }
    }

    for (std::size_t i = 0; i < vec.size(); i++ ) {
        const float a = vec.at(i);
        rectangles.at(i).setFillColor(sf::Color(a * 200, a * 200, a * 200));
        if (i == selectedIndex){
            rectangles.at(i).setOutlineColor(sf::Color::Red);
        }
        window->draw(rectangles.at(i));
        if (i == selectedIndex){
            rectangles.at(i).setOutlineColor(sf::Color(50, 50, 50));
        }
    }
}

void GUI::VectorRenderer::drawCorr(sf::RenderWindow *window, const std::vector<float> &vec, size_t selectedIndex) {
    if (vec.size() != rectangles.size()){
        rectangles.clear();
        const auto c = vec.size();
        rectangles.reserve(c);
        for (std::size_t i = 0; i < c; i++){
            rectangles.emplace_back(sf::Vector2f(bounds.width/c, bounds.height));
            rectangles.back().setPosition(bounds.left + i*(bounds.width/c+2.f), bounds.top);
            rectangles.back().setOutlineThickness(1);
            rectangles.back().setOutlineColor(sf::Color(50, 50, 50));
        }
    }

    for (std::size_t i = 0; i < vec.size(); i++ ){
        const float val = (vec.at(i) + 1.f)/2.f;
        const float o = std::fabs(vec.at(i));
        rectangles.at(i).setFillColor(sf::Color(val*200, val*200, val*200, o*255));
        if (i == selectedIndex){
            rectangles.at(i).setOutlineColor(sf::Color::Red);
        }
        window->draw(rectangles.at(i));
        if (i == selectedIndex){
            rectangles.at(i).setOutlineColor(sf::Color(50, 50, 50));
        }
    }
}
