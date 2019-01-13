//
// Created by Axel on 2019-01-10.
//

#include <chrono>
#include <fstream>

#include "Config.h"

void Config::loadConfigFromFile(const std::string &filename) {
    std::ifstream file(filename);
    if (file.fail()){
        throw std::runtime_error("Can't open config file: "+filename+"\n");
    }
    nlohmann::json json;
    file >> json;
    file.close();

    // Loading seed

    if (json["seed"].is_string()){
        if (json["seed"].get<std::string>() == "TIME"){
            Config::seed = static_cast<unsigned>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
        }
    }
    else {
        Config::seed = json["seed"].get<unsigned>();
    }

    // Loading World settings
    auto &WS = json["WorldSettings"];
    auto worldWidth = WS["worldWidth"].get<float>();
    auto worldHeight = WS["worldHeight"].get<float>();
    world.dimensions = {worldWidth, worldHeight};
    world.terrainSquare = WS["terrainSquare"].get<unsigned>();
    world.quadtreeLimit = WS["quadtreeLimit"].get<float>();

    auto &populatorEntries = WS["PopulatorEntries"];
    for (auto &entry : populatorEntries){
        world.populatorEntries.push_back({
        .type = entry["type"].get<std::string>(),
        .count = 0,
        .targetCount = entry["targetCount"].get<unsigned>(),
        .rate = entry["rate"].get<float>(),
        .enabled = true
        });
    }

    // Loading agent settings
    auto &AS = json["AgentSettings"];
    agents.mass = AS["mass"].get<float>();
    agents.friction = AS["friction"].get<float>();
    agents.maxSpeed = AS["maxSpeed"].get<float>();
    agents.turnFactor = AS["turnFactor"].get<float>();
    agents.actionCooldown = AS["actionCooldown"].get<float>();

    agents.energyLossRate = AS["energyLossRate"].get<float>();
    agents.movementEnergyLoss = AS["movementEnergyLoss"].get<float>();
    agents.mushroomEnergy = AS["mushroomEnergy"].get<float>();
    agents.maxEnergy = AS["maxEnergy"].get<float>();

    agents.canReproduce = AS["canReproduce"].get<bool>();
    agents.canWalk = AS["canWalk"].get<bool>();
    agents.canTurn = AS["canTurn"].get<bool>();
    agents.canEat = AS["canEat"].get<bool>();
    agents.canPlace = AS["canPlace"].get<bool>();

    agents.memory = AS["memory"].get<unsigned>();

    agents.perceiveCollision = AS["perceiveCollision"].get<bool>();
    agents.receptorCount = AS["receptorCount"].get<unsigned>();
    agents.perceiveColor = AS["perceiveColor"].get<bool>();
    agents.perceiveEnergyLevel = AS["perceiveEnergyLevel"].get<bool>();

    agents.FOV = AS["FOV"].get<float>();
    agents.visibilityDistance = AS["visibilityDistance"].get<float>();
    agents.visualReactivity = AS["visualReactivity"].get<float>();

    agents.mutation = AS["mutation"].get<float>();
    agents.layersMin = AS["layerMin"].get<int>(); agents.layersMax = AS["layerMax"].get<int>();
    agents.biasMin = AS["biasMin"].get<float>(); agents.biasMax = AS["biasMax"].get<float>();
    agents.weightMin = AS["weightMin"].get<float>(); agents.weightMax = AS["weightMax"].get<float>();
    agents.perceptronPerLayerMin = AS["perceptronPerLayerMin"].get<int>();
    agents.perceptronPerLayerMax = AS["perceptronPerLayerMax"].get<int>();


    // Loading controls
    auto &C = json["Controls"];
    controls.pause = findKeyCode(C["pause"].get<std::string>());
    controls.close = findKeyCode(C["close"].get<std::string>());
    controls.showDebug = findKeyCode(C["showDebug"].get<std::string>());
    controls.up = findKeyCode(C["up"].get<std::string>());
    controls.down = findKeyCode(C["down"].get<std::string>());
    controls.left = findKeyCode(C["left"].get<std::string>());
    controls.right = findKeyCode(C["right"].get<std::string>());
    controls.slowDown = findKeyCode(C["slowDown"].get<std::string>());
    controls.speedUp = findKeyCode(C["speedUp"].get<std::string>());

    controls.upAmount = -C["keyboardCameraMove"].get<float>();
    controls.downAmount = C["keyboardCameraMove"].get<float>();
    controls.leftAmount = -C["keyboardCameraMove"].get<float>();
    controls.rightAmount = C["keyboardCameraMove"].get<float>();
    controls.timeFactorDelta = C["timeFactorDelta"].get<float>();
    controls.timeFactorMax = C["timeFactorMax"].get<float>();
    controls.scrollFactor = C["scrollFactor"].get<float>();

    auto &RS = json["Rendering"];
    render.showDistribution = RS["showDistribution"].get<bool>();
    render.bins = RS["bins"].get<unsigned>();
    render.showWorldObjectBounds = RS["showWorldObjectBounds"].get<bool>();
    render.showDebug = RS["showDebug"].get<bool>();
    render.showQuadtree = RS["showQuadtree"].get<bool>();
    render.showQuadtreeEntities = RS["showQuadtreeEntities"].get<bool>();
    render.showVision = RS["showVision"].get<bool>();
    render.renderGeneration = RS["renderGeneration"].get<bool>();

    auto windowWidth = RS["windowWidth"].get<unsigned>();
    auto windowHeight = RS["windowHeight"].get<unsigned>();
    render.windowSize = {windowWidth, windowHeight};
}


sf::Keyboard::Key Config::findKeyCode(std::string key) {
    static const std::map<std::string, sf::Keyboard::Key> m { // TODO: add the rest
        {"Space", sf::Keyboard::Space},
        {"Escape", sf::Keyboard::Escape},
        {"D", sf::Keyboard::D},
        {"Up", sf::Keyboard::Up},
        {"Down", sf::Keyboard::Down},
        {"Left", sf::Keyboard::Left},
        {"Right", sf::Keyboard::Right},
        {"Comma", sf::Keyboard::Comma},
        {"Period", sf::Keyboard::Period}
    };

    return m.at(key);
}
