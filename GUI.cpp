//
// Created by axelw on 2019-01-06.
//


#include <sstream>
#include <utility>
#include <random>

#include "GUI.h"
#include "World.h"
#include "Camera.h"

GUI::GUI(Config &config, sf::RenderWindow *window, World *world, Camera *camera)
        : config(config), window(window), originalWindowSize(window->getSize()), view(window->getDefaultView()),
        world(world), camera(camera) {
    font.loadFromFile(R"(C:\Windows\Fonts\consola.ttf)");
    sf::Color gray(120, 120, 120);

    simulationInfo.main = sf::Text(std::string("FPS: ###\nTime factor: ##\nPopulation: ###\nMushrooms: ###\n"), font);
    simulationInfo.main.setCharacterSize(20);
    simulationInfo.main.setStyle(sf::Text::Regular);
    simulationInfo.main.setFillColor(gray);
    simulationInfo.main.setPosition(10, window->getSize().y-simulationInfo.main.getLocalBounds().height-5);


    simulationInfo.debug = {
        Toggle("agentSpawning", &world->agentSpawning),
        Toggle("reloadConfig", &config.shouldReload),
        Toggle("showWorldObjectBounds", &config.render.showWorldObjectBounds),
        Toggle("showQuadtree", &config.render.showQuadtree),
        Toggle("showQuadtreeEntities", &config.render.showQuadtreeEntities),
        Toggle("showVision", &config.render.showVision),
        Toggle("showPaths", &config.render.showPaths),
        Toggle("graphLine", &config.render.graphLine),
        Toggle("graphSpectrogram", &config.render.graphSpectrogram),
        Toggle("renderOnlyAgents", &config.render.renderOnlyAgents),
        Toggle("visualizeGeneration", &config.render.visualizeGeneration),
        Toggle("visualizeAge", &config.render.visualizeAge),
        Toggle("visualizeChildren", &config.render.visualizeChildren),
        Toggle("visualizeMurders", &config.render.visualizeMurders),
        Toggle("visualizeMushrooms", &config.render.visualizeMushrooms),
        Toggle("showSquare", &config.render.visualizeColor)
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

        if (toggle.text.getString().toAnsiString() == "graphLine"){
            toggle.subToggles = {
                    Toggle("population", &config.render.graphPopulation, &toggle),
                    Toggle("mean gen.", &config.render.graphMeanGeneration, &toggle),
                    Toggle("mean perceptrons", &config.render.graphMeanPerceptrons, &toggle),
                    Toggle("mean age", &config.render.graphMeanAge, &toggle),
                    Toggle("mean children", &config.render.graphMeanChildren, &toggle),
                    Toggle("mean murders", &config.render.graphMeanMurders, &toggle),
                    Toggle("mean energy", &config.render.graphMeanEnergy, &toggle),
                    Toggle("mean mushrooms", &config.render.graphMeanMushrooms, &toggle),
                    Toggle("mean speed", &config.render.graphMeanSpeed, &toggle)
            };

            for (std::size_t j = 0; j < toggle.subToggles.size(); j++){
                toggle.subToggles.at(j).text.setPosition(
                        window->getSize().x-420,
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
                    .yPixelOffset = (unsigned) j,
                    .valueText = sf::Text("###", font, 12)
                });
                lineGraphs.back().valueText.setFillColor(sf::Color(col[0], col[1], col[2]));
            }
        }
        else if (toggle.text.getString().toAnsiString() == "graphSpectrogram"){
                toggle.subToggles = {
                        Toggle("generation", &config.render.graphGeneration, &toggle),
                        Toggle("perceptrons", &config.render.graphPerceptrons, &toggle),
                        Toggle("age", &config.render.graphAge, &toggle),
                        Toggle("children", &config.render.graphChildren, &toggle),
                        Toggle("murders", &config.render.graphMurders, &toggle),
                        Toggle("energy", &config.render.graphEnergy, &toggle),
                        Toggle("mushrooms", &config.render.graphMushrooms, &toggle)
                };

                auto blank = sf::Color(0, 0, 0, 0);
                spectrograms = {
                        (Spectrogram) {.name = "generation", .shouldRender=&config.render.graphGeneration,
                                       .stride=2, .markerWidth=1, .startHeight=10, .spectrogram=Contiguous2dVector(blank)},
                        (Spectrogram) {.name = "perceptrons", .shouldRender=&config.render.graphPerceptrons,
                                .stride=5, .markerWidth=8, .startHeight=80, .spectrogram=Contiguous2dVector(blank)},
                        (Spectrogram) {.name = "age", .shouldRender=&config.render.graphAge,
                                .stride=0.1f, .markerWidth=5, .startHeight=100, .spectrogram=Contiguous2dVector(blank)},
                        (Spectrogram) {.name = "children", .shouldRender=&config.render.graphChildren,
                                .stride=1, .markerWidth=1, .startHeight=20, .spectrogram=Contiguous2dVector(blank)},
                        (Spectrogram) {.name = "murders", .shouldRender=&config.render.graphMurders,
                                .stride=1, .markerWidth=1, .startHeight=20, .spectrogram=Contiguous2dVector(blank)},
                        (Spectrogram) {.name = "energy", .shouldRender=&config.render.graphEnergy,
                                .stride=0.5, .markerWidth=4, .startHeight=100, .spectrogram=Contiguous2dVector(blank)},
                        (Spectrogram) {.name = "mushrooms", .shouldRender=&config.render.graphMushrooms,
                                .stride=1, .markerWidth=3, .startHeight=20, .spectrogram=Contiguous2dVector(blank)}
                };

                for (std::size_t j = 0; j < toggle.subToggles.size(); j++){
                    toggle.subToggles.at(j).text.setPosition(
                            window->getSize().x-420,
                            toggle.text.getPosition().y+j*12
                    );
                    toggle.subToggles.at(j).text.setCharacterSize(15);
                    toggle.subToggles.at(j).text.setFont(font);
                    toggle.subToggles.at(j).update();
                    toggle.exclusiveSubs = true;
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
    if (!config.render.showInterface)
        return;
    // UI camera-view
    auto cameraView = window->getView();
    window->setView(view);

    { // Draw current world statistics
        simulationInfo.main.setString("FPS: " + std::to_string(int(1.f / deltaTime))
                                      + "\nTime factor: " + std::to_string(int(timeFactor))
                                      + "\nPopulation: " + std::to_string(world->getAgents().size())
                                      + "\nMushrooms: " + std::to_string(world->getHistoricalStatistics().back().mushroomCount)
                                      +"\n");

        window->draw(simulationInfo.main);
    }

    if (config.render.graphLine){
        for (auto &lg : lineGraphs){
            if (!*lg.shouldRender) continue;
            lg.update(world);
            lg.draw(window, (sf::Vector2f) originalWindowSize);
        }
    }

    // Draw spectrogram
    if (config.render.graphSpectrogram){
        for (auto &sp : spectrograms){
            if (!*sp.shouldRender) continue;
            sp.update(world);
            sp.draw(window, (sf::Vector2f) originalWindowSize);
        }
    }

    // Draw debug info
    for (auto& t : simulationInfo.debug){
        t.update();
        window->draw(t.text);

        if (t.hovered){
            for (auto &sub : t.subToggles){
                window->draw(sub.text);
            }
        }
    }


    // Draw information about the selected agent (if there is one)
    {
        if (selectedAgent) {
            selectedAgent->queuePathDraw();
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

    if (config.render.showInterface){
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
                else if (t.text.getString().substring(0, 5) == "graph") {
                    for (auto& c : simulationInfo.debug){
                        if (c.text.getString() != t.text.getString() && c.text.getString().substring(0, 5) == "graph"){
                            c.set(false);
                            c.hovered = false;
                        }
                    }
                }
                return true;
            }
            if (t.hovered) {
                for (auto &sub : t.subToggles) {
                    if (pointInBox(sf::Vector2f(pos.x, pos.y), sub.text.getGlobalBounds())) {
                        sub.click();
                        return true;
                    }
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

    if (config.render.showInterface){
        std::function<bool(Toggle&)> hoverToggle = [&](Toggle &t) -> bool {
            bool anyHovered = false;
            if (t.hovered){
                for (auto &sub : t.subToggles){
                    anyHovered |= hoverToggle(sub);
                }
            }
            sf::FloatRect bounds = t.text.getGlobalBounds();
            if (t.parent != nullptr){
                bounds.width += 40;
                bounds.height += 40;
            }
            if (anyHovered || pointInBox(sf::Vector2f(pos.x, pos.y), bounds)){
                t.hovered = true;
                return true;
            }
            t.hovered = false;
            return false;
        };

        for (auto &toggle : simulationInfo.debug){
            bool h = hoverToggle(toggle);
            if (h && toggle.text.getString().substring(0, 5) == "graph") {
                for (auto& c : simulationInfo.debug){
                    if (c.text.getString() != toggle.text.getString() && c.text.getString().substring(0, 5) == "graph"){
                        c.hovered = false;
                    }
                }
            }
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

void GUI::LineGraph::update(const World *world){
    auto &stats = world->getHistoricalStatistics();
    if (stats.empty())
        return;

    if (stats.size() <= verts.getVertexCount()){
        verts.clear();
        lastUpdateFrame = 0;
    }

    for (std::size_t i = lastUpdateFrame; i < stats.size(); i++){
        const auto &s = stats[i];
        float x, y;
        x = s.timestamp;
        if (name == "population")
            y = s.populationCount;
        else if (name == "mean gen.")
            y = (float) std::accumulate(std::begin(s.generation), std::end(s.generation), 0,
                    [](float acc, const WorldStatistics::ColorValue& value){ return acc+value.value; })
                / s.populationCount;
        else if (name == "mean perceptrons")
            y = (float) std::accumulate(std::begin(s.perceptrons), std::end(s.perceptrons), 0,
                    [](float acc, const WorldStatistics::ColorValue& value){ return acc+value.value; })
                / s.populationCount;
        else if (name == "mean age")
            y = (float) std::accumulate(std::begin(s.age), std::end(s.age), 0,
                    [](float acc, const WorldStatistics::ColorValue& value){ return acc+value.value; })
                / s.populationCount;
        else if (name == "mean children")
            y = (float) std::accumulate(std::begin(s.children), std::end(s.children), 0,
                    [](float acc, const WorldStatistics::ColorValue& value){ return acc+value.value; })
                / s.populationCount;
        else if (name == "mean murders")
            y = (float) std::accumulate(std::begin(s.murders), std::end(s.murders), 0,
                                        [](float acc, const WorldStatistics::ColorValue& value){ return acc+value.value; })
                / s.populationCount;
        else if (name == "mean energy")
            y = (float) std::accumulate(std::begin(s.energy), std::end(s.energy), 0,
                                        [](float acc, const WorldStatistics::ColorValue& value){ return acc+value.value; })
                / s.populationCount;
        else if (name == "mean mushrooms")
            y = (float) std::accumulate(std::begin(s.mushrooms), std::end(s.mushrooms), 0,
                    [](float acc, const WorldStatistics::ColorValue& value){ return acc+value.value; })
                / s.populationCount;
        else if (name == "mean speed")
            y = (float) std::accumulate(std::begin(s.speed), std::end(s.speed), 0,
                                        [](float acc, const WorldStatistics::ColorValue& value){ return acc+value.value; })
                / s.populationCount;
        else
            throw std::runtime_error("Line Graph datum "+name+" doesn't exist");

        y *= -1;
        min = sf::Vector2f(std::min(min.x, x), std::min(min.y, y));
        max = sf::Vector2f(std::max(max.x, x), std::max(max.y, y));

        verts.append(sf::Vertex({x, y}, color));
    }
    lastUpdateFrame = stats.size();
}

void GUI::LineGraph::draw(sf::RenderWindow *window, const sf::Vector2f orgSize) {
    // Set all the vertex data
    verts.setPrimitiveType(sf::LineStrip);

    // Draw graph
    sf::View view;
    view.reset(sf::FloatRect(sf::Vector2f(-2, -2) + min, max-min + sf::Vector2f(2, 2) ));
    view.setViewport(sf::FloatRect(0.1, 0.75, 0.8, 0.24));
    auto oldView = window->getView();
    window->setView(view);
    window->draw(verts);

    // Draw value in end
    sf::Vector2f lastPos = verts[verts.getVertexCount()-1].position;
    lastPos = view.getTransform().transformPoint(lastPos);
    lastPos = sf::Vector2f(lastPos.x*view.getViewport().width, -lastPos.y*view.getViewport().height);
    lastPos += sf::Vector2f(view.getViewport().left, view.getViewport().top+view.getViewport().height);
    lastPos = sf::Vector2f(lastPos.x*orgSize.x, lastPos.y*orgSize.y);
    if (!valueText.getString().isEmpty() && lastPos.x == lastPos.x && lastPos.y == lastPos.y){
        valueText.setPosition(lastPos);
        valueText.setString(std::to_string((int) verts[verts.getVertexCount()-1].position.y)+" "+name);
        window->draw(valueText);
    }
    window->setView(oldView);
}

void GUI::Spectrogram::update(const World *world) {
    auto &stats = world->getHistoricalStatistics();
    if (stats.empty())
        return;

    if (stats.size() < currentSize.x){
        spectrogram.clear();
    }

    if (spectrogram.getN() == 0 && spectrogram.getM() == 0){
        // Allocated the whole spectrogram
        spectrogram = Contiguous2dVector(downsamplingTriggerW, downsamplingTriggerH,
                sf::Color(0, 0, 0, 0));
        currentSize = sf::Vector2u(0, startHeight);
        lastUpdateFrame = 0;
        perRow = 1;
        perColumn = 1; columnCounter = 0;
    }

    std::size_t j = 0;
    for (std::size_t i = lastUpdateFrame; i < stats.size(); i++){
        const auto &s = stats.at(i);
        lastUpdateFrame++; j++;
        if (16 < j)
            break;

        std::vector<WorldStatistics::ColorValue> values;
        values.resize(s.populationCount);

        if (name == "generation")
            values = s.generation;
        else if (name == "perceptrons")
            values = s.perceptrons;
        else if (name == "age")
            values = s.age;
        else if (name == "children")
            values = s.children;
        else if (name == "murders")
            values = s.murders;
        else if (name == "energy")
            values = s.energy;
        else if (name == "mushrooms")
            values = s.mushrooms;
        else
            throw std::runtime_error("Spectrogram Graph datum "+name+" doesn't exist");

        auto minIt = std::min_element(std::begin(values), std::end(values),
                [](const WorldStatistics::ColorValue& a, const WorldStatistics::ColorValue& b){
                    return a.value < b.value;
        });

        auto maxIt = std::max_element(std::begin(values), std::end(values),
                [](const WorldStatistics::ColorValue& a, const WorldStatistics::ColorValue& b){
            return a.value < b.value;
        });

        if (minIt != std::end(values))
            minVal = std::fminf(minVal, float(*minIt));
        if (maxIt != std::end(values))
            maxVal = std::fmaxf(maxVal, float(*maxIt));

        newValues.push_back(values);
    }
    
    // Lambda for drawing in spectrogram
    auto mark = [](std::vector<std::array<unsigned, 4>> &vec,
            std::vector<float> &totals,
            const float ind, const sf::Color color,
            const float opacity, const unsigned size) {

        for (std::size_t i = ind-std::floor((float) size/2); i < ind+std::ceil((float) size/2); i++) {
            try {
                float dist = 1.5f/(fabsf(ind-i)+1.5f);
                vec.at(i).at(0) += color.r;
                vec.at(i).at(1) += color.g;
                vec.at(i).at(2) += color.b;
                vec.at(i).at(3) += opacity*255.f*dist;
                totals.at(i) += 1.0f;
            } catch (const std::out_of_range& e) {};
        }
    };

    // Go through each value and draw
    for (auto& values : newValues){
        unsigned currentY = (maxVal-minVal) * stride / (float) perRow;
        if (downsamplingTriggerH <= currentY){
            // Half existing column
            printf("Halving color column\n");
            std::vector<sf::Color> newColColumn(std::ceil(colorColumn.size()/2.f));
            std::vector<unsigned> newColCount(std::ceil(colorColumnCount.size()/2.f));
            for (std::size_t y = 0; y < newColColumn.size(); y++) {
                unsigned r = 0, g = 0, b = 0, a = 0;

                float total = 0;
                std::vector<sf::Color> colors = {colorColumn.at(y*2)};
                std::vector<unsigned> colorsCount = {colorColumnCount.at(y*2)};
                if (y*2+1 < colorColumn.size()){
                    colors.push_back(colorColumn.at(y*2+1));
                    colorsCount.push_back(colorColumnCount.at(y*2+1));
                }
                else {
                    printf("Avoid color %zu\n", y*2+1);
                }


                for (auto &c : colors){
                    if (c.r == 0 && c.g == 0 && c.b == 0 && c.a == 0){
                        total += 0;
                    }
                    else {
                        r += c.r;
                        g += c.g;
                        b += c.b;
                        a += c.a;
                        total += 1;
                    }
                }


                unsigned colCount = 0;
                for (auto &c : colorsCount){
                    colCount += c;
                }

                r /= total; g /= total;
                b /= total; a /= total;
                newColColumn.at(y) = sf::Color(r, g, b, a);
                newColCount.at(y) = colCount;
            }

            printf("Halfing spectrogram height\n");
            // Half height
            auto newSpec = Contiguous2dVector(spectrogram.getN(), spectrogram.getM(), spectrogram.getFillValue());
            for (std::size_t x = 0; x < currentSize.x; x++) {
                for (std::size_t y = 0; y < std::ceil(newSpec.getM()/2)-1; y++) {
                    unsigned r = 0, g = 0, b = 0, a = 0;
                    float total = 0;
                    std::vector<sf::Color> colors;
                    try {
                        colors = {spectrogram.at(x, y*2)};
                    } catch (const std::out_of_range &e){
                        printf("Is outarange a\n");
                    }
                    if (y*2+1 < spectrogram.getM()){
                        try {
                            colors.push_back(spectrogram.at(x, y * 2 + 1));
                        } catch (const std::out_of_range &e) {
                            printf("Outarange b\n");
                        }
                    }
                    for (auto &c : colors){
                        if (c.r == 0 && c.g == 0 && c.b == 0 && c.a == 0){
                            total += 0;
                        }
                        else {
                            r += c.r;
                            g += c.g;
                            b += c.b;
                            a += c.a;
                            total += 1;
                        }
                    }
                    r /= total; g /= total;
                    b /= total; a /= total;
                    try {
                        newSpec.at(x, y) = sf::Color(r, g, b, a);
                    } catch (const std::out_of_range &e){
                        printf("IS outarange b\n");
                    }
                }
            }

            spectrogram = newSpec;
            perRow *= 2;
            currentSize.y = std::ceil(downsamplingTriggerH/2.0);

        }
        try {
            std::vector<std::array<unsigned, 4>> column(currentY);

            std::vector<float> totals(column.size(), 0.f);
            // Global color column has to be of sufficient size
            if (colorColumn.size() < column.size()) {
                colorColumn.resize(column.size(), sf::Color(0, 0, 0, 0));
                colorColumnCount.resize(column.size(), 0);
            }

            // Draw values to column
            for (const auto &value : values) {
                std::size_t ind = (value.value - minVal) * (float) stride / (float) perRow;
                mark(column, totals, ind, value.color, .8f, markerWidth);
            }

            // Divide in column to average colors
            for (std::size_t i = 0; i < column.size(); i++) {
                column.at(i).at(0) /= totals.at(i);
                column.at(i).at(1) /= totals.at(i);
                column.at(i).at(2) /= totals.at(i);
                column.at(i).at(3) /= totals.at(i);
                colorColumnCount.at(i)++;
            }

            // Add to spectrogram
            columnCounter++;
            if (columnCounter == perColumn) {
                // Rescale the column if it too big
                for (std::size_t i = 0; i < column.size(); i++) {
                    if (colorColumnCount.at(i) != 0) {
                        colorColumn.at(i).r += column.at(i).at(0);
                        colorColumn.at(i).g += column.at(i).at(1);
                        colorColumn.at(i).b += column.at(i).at(2);
                        colorColumn.at(i).a += column.at(i).at(3);
                        colorColumnCount.at(i) = 0;
                    }
                }

                auto p = spectrogram.at(currentSize.x);
                auto itCol = p.first;
                for (auto it = std::begin(colorColumn); it != std::end(colorColumn); it++) {
                    *itCol = *it;
                    itCol++;
                }

                currentSize.x += 1;
                currentSize.y = std::max((std::size_t) currentSize.y, colorColumn.size());
                colorColumn.clear();
                colorColumnCount.clear();
                columnCounter = 0;
            }

            // Check if spectrogram is too wide
            if (spectrogram.getN() <= currentSize.x) {
                // Half width
                auto newSpec = Contiguous2dVector(spectrogram.getN(), spectrogram.getM(), spectrogram.getFillValue());
                for (std::size_t x = 0; x < std::ceil(newSpec.getN() / 2.0); x++) {
                    for (std::size_t y = 0; y < newSpec.getM(); y++) {
                        unsigned r = 0, g = 0, b = 0, a = 0;
                        float total = 0;
                        for (auto &c : {spectrogram.at(x * 2, y), spectrogram.at(x * 2 + 1, y)}) {
                            if (c.r == 0 && c.g == 0 && c.b == 0 && c.a == 0) {
                                total += 0;
                            } else {
                                r += c.r;
                                g += c.g;
                                b += c.b;
                                a += c.a;
                                total += 1;
                            }
                        }
                        r /= total;
                        g /= total;
                        b /= total;
                        a /= total;
                        newSpec.at(x, y) = sf::Color(r, g, b, a);
                    }
                }

                spectrogram = newSpec;
                perColumn *= 2;
                currentSize.x = std::ceil(currentSize.x / 2.0);
            }
        } catch (const std::out_of_range &e){
            printf("Other outarange\n");
        }
    }

    newValues.clear();
}

void GUI::Spectrogram::draw(sf::RenderWindow *window, const sf::Vector2f orgSize) {
    if (currentSize.x == 0 || currentSize.y == 0)
        return;

    sf::Image image;
    image.create(currentSize.x, currentSize.y);

    for (std::size_t x = 0; x < image.getSize().x; x++){
        for (std::size_t y = 0; y < image.getSize().y; y++){
            image.setPixel(x, y, spectrogram.at(x, y));
        }
    }

    sf::Texture texture;
    texture.loadFromImage(image);
    auto sprite = sf::Sprite(texture);

    sf::View view;
    view.reset(sf::FloatRect(0, 0, currentSize.x, currentSize.y));
    view.setViewport(sf::FloatRect(0.1, 0.75, 0.8, 0.24));
    view.setSize(view.getSize().x, -view.getSize().y);
    auto oldView = window->getView();
    window->setView(view);

    window->draw(sprite);
    window->setView(oldView);
}

GUI::Toggle::Toggle(const std::string& name, bool *value, std::vector<Toggle> subToggles, sf::Color color) :
color(color){
    text.setString(name);
    text.setCharacterSize(20);

    Toggle::value = value;
    Toggle::subToggles = std::move(subToggles);
    Toggle::hovered = false;
    Toggle::parent = nullptr;

    update();
}

GUI::Toggle::Toggle(const std::string& name, bool *value, Toggle* parent, sf::Color color) :
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
        if (parent != nullptr){
            if (parent->exclusiveSubs){
                for (auto &sub : parent->subToggles){
                    if (&sub != this){
                        sub.set(false);
                    }
                }
            }
        }
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

