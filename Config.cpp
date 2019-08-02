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

    shouldReload = false;

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

    world.mushroomReproductionRate = WS["mushroomReproductionRate"].get<float>();
    world.mushroomReproductionDistance = WS["mushroomReproductionDistance"].get<float>();
    world.mushroomReproductionNearLimit = WS["mushroomReproductionNearLimit"].get<unsigned>();

    world.dimensions = {worldWidth, worldHeight};
    world.terrainSquare = WS["terrainSquare"].get<unsigned>();
    world.quadtreeLimit = WS["quadtreeLimit"].get<float>();

    auto &populatorEntries = WS["PopulatorEntries"];
    for (auto &entry : populatorEntries){
        auto type = entry["type"].get<std::string>();
        if (world.populatorEntries.find(type) == world.populatorEntries.end()){
            Populator::Entry e = {
                .type = type,
                .count = 0,
                .enabled = true
            };
            world.populatorEntries.insert(std::make_pair(type, e));
        }

        auto existingEntry = world.populatorEntries.find(type);
        existingEntry->second.targetCount = entry["targetCount"].get<unsigned>();
        existingEntry->second.rate = entry["rate"].get<float>();
    }

    for (auto &entry : world.populatorEntries){
        bool found = false;
        for (auto &configEntry : populatorEntries){
            if (entry.first == configEntry["type"].get<std::string>()){
                found = true;
                break;
            }
        }

        if (!found){
            world.populatorEntries.erase(entry.first);
        }
    }


    // Loading agent settings
    auto &AS = json["AgentSettings"];
    agents.mass = AS["mass"].get<float>();
    agents.friction = AS["friction"].get<float>();
    agents.maxSpeed = AS["maxSpeed"].get<float>();
    agents.turnFactor = AS["turnFactor"].get<float>();
    agents.punchTime = AS["punchTime"].get<float>();
    agents.actionCooldown = AS["actionCooldown"].get<float>();

    agents.energyToParent = AS["energyToParent"].get<float>();
    agents.energyToChild = AS["energyToChild"].get<float>();
    agents.energyLossRate = AS["energyLossRate"].get<float>();
    agents.turnRateEnergyLoss = AS["turnRateEnergyLoss"].get<float>();
    agents.movementEnergyLoss = AS["movementEnergyLoss"].get<float>();
    agents.punchEnergy = AS["punchEnergy"].get<float>();
    agents.punchDamage = AS["punchDamage"].get<float>();
    agents.mushroomEnergy = AS["mushroomEnergy"].get<float>();
    agents.maxEnergy = AS["maxEnergy"].get<float>();
    agents.maxMushroomCount = AS["maxMushroomCount"].get<unsigned>();

    agents.canReproduce = AS["canReproduce"].get<bool>();
    agents.canWalk = AS["canWalk"].get<bool>();
    agents.canTurn = AS["canTurn"].get<bool>();
    agents.canEat = AS["canEat"].get<bool>();
    agents.canPlace = AS["canPlace"].get<bool>();
    agents.canPunch = AS["canPunch"].get<bool>();

    agents.memory = AS["memory"].get<unsigned>();
    agents.memoryReactivity = AS["memoryReactivity"].get<float>();

    agents.perceiveCollision = AS["perceiveCollision"].get<bool>();
    agents.receptorCount = AS["receptorCount"].get<unsigned>();
    agents.perceiveColor = AS["perceiveColor"].get<bool>();
    agents.perceiveEnergyLevel = AS["perceiveEnergyLevel"].get<bool>();
    agents.perceiveMushroomCount = AS["perceiveMushroomCount"].get<bool>();

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
    controls.clearStats = findKeyCode(C["clearStats"].get<std::string>());
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
    controls.timeFactorInitial = C["timeFactorInitial"].get<float>();
    controls.timeFactorDelta = C["timeFactorDelta"].get<float>();
    controls.timeFactorMax = C["timeFactorMax"].get<float>();
    controls.scrollFactor = C["scrollFactor"].get<float>();

    auto &RS = json["Rendering"];
    render.graphLine = RS["graphLine"].get<bool>();
        render.graphPopulation = RS["graphPopulation"].get<bool>();
        render.graphMeanGeneration = RS["graphMeanGeneration"].get<bool>();
        render.graphMeanPerceptrons = RS["graphMeanPerceptrons"].get<bool>();
        render.graphMeanAge = RS["graphMeanAge"].get<bool>();
        render.graphMeanChildren = RS["graphMeanChildren"].get<bool>();
        render.graphMeanMurders = RS["graphMeanMurders"].get<bool>();
        render.graphMeanEnergy = RS["graphMeanEnergy"].get<bool>();
        render.graphMeanMushrooms = RS["graphMeanMushrooms"].get<bool>();
        render.graphMeanSpeed = RS["graphMeanSpeed"].get<bool>();

    render.graphSpectrogram = RS["graphSpectrogram"].get<bool>();
        render.graphGeneration = RS["graphGeneration"].get<bool>();
        render.graphPerceptrons = RS["graphPerceptrons"].get<bool>();
        render.graphAge = RS["graphAge"].get<bool>();
        render.graphEnergy = RS["graphEnergy"].get<bool>();
        render.graphChildren = RS["graphChildren"].get<bool>();
        render.graphMurders = RS["graphMurders"].get<bool>();
        render.graphMushrooms = RS["graphMushrooms"].get<bool>();
        render.graphSpeed = RS["graphSpeed"].get<bool>();

    render.bins = RS["bins"].get<unsigned>();
    render.showWorldObjectBounds = RS["showWorldObjectBounds"].get<bool>();
    render.showDebug = RS["showDebug"].get<bool>();
    render.showQuadtree = RS["showQuadtree"].get<bool>();
    render.showQuadtreeEntities = RS["showQuadtreeEntities"].get<bool>();
    render.showVision = RS["showVision"].get<bool>();
    render.renderOnlyAgents = false;
    render.visualizeGeneration = false;
    render.visualizeAge = false;
    render.visualizeMushrooms = false;
    render.visualizeChildren = false;
    render.visualizeMurders = false;
    render.visualizeColor = false;

    auto windowWidth = RS["windowWidth"].get<unsigned>();
    auto windowHeight = RS["windowHeight"].get<unsigned>();
    render.windowSize = {windowWidth, windowHeight};
}


sf::Keyboard::Key Config::findKeyCode(std::string key) {
    static const std::map<std::string, sf::Keyboard::Key> m { // TODO: add the rest
        {"Space", sf::Keyboard::Space},
        {"Escape", sf::Keyboard::Escape},
        {"D", sf::Keyboard::D},
        {"C", sf::Keyboard::C},
        {"Up", sf::Keyboard::Up},
        {"Down", sf::Keyboard::Down},
        {"Left", sf::Keyboard::Left},
        {"Right", sf::Keyboard::Right},
        {"Comma", sf::Keyboard::Comma},
        {"Period", sf::Keyboard::Period}
    };

    return m.at(key);
}
