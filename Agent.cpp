//
// Created by Axel on 2018-11-21.
//

#include <iostream>
#include <cmath>

#include "Agent.h"
#include "Config.h"
#include "utils.cpp"
#include "World.h"
#include "Mushroom.h"
#include "MarkovNames.h"

#define PI 3.14159265f

bool Agent::loaded = false;
sf::Texture Agent::walkingTexture;

void Agent::loadResources() {
    if (!loaded){
        Agent::walkingTexture.loadFromFile("resources/WalkCycle.png");
        loaded = true;
    }
}

Agent::Agent(const AgentSettings &settings, World *world, sf::Vector2f position, float orientation)
        : WorldObject("Agent", world, position, true),
          orientation(orientation) {
    loadResources();

    generation = 0;
    maxEnergy = 100;
    energy = maxEnergy;
    energyLossRate = settings.energyLossRate;
    setMass(settings.mass);
    setFriction(settings.friction);
    turnFactor = settings.turnFactor;
    maxSpeed = settings.maxSpeed;

    frameIndex = 0;
    frame = sf::IntRect(0, 0, 32, 32);
    sprite = sf::Sprite(walkingTexture, frame);
    sprite.setOrigin(16, 5);
    setBounds(sf::IntRect(-8, 10, 8, 27));

    receptorCount = settings.receptorCount;
    FOV = settings.FOV;
    visibilityDistance = settings.visibilityDistance;
    visualReactivity = settings.visualReactivity;

    receptors.resize(receptorCount);
    std::fill(std::begin(receptors), std::end(receptors), 0.f);

    lineOfSight.resize(receptorCount*2);
    lineOfSight[0] = sf::Vertex(sf::Vector2f(0,0));
    lineOfSight[1] = sf::Vertex(sf::Vector2f(0,0));
    lineOfSight[0].color = sf::Color::Cyan;
    lineOfSight[1].color = sf::Color::Cyan;

    std::size_t outputCount = 4;
    std::size_t inputCount = receptors.size() + 1;

    percept = std::vector<float>(inputCount);
    std::fill(std::begin(percept), std::end(percept), 0.f);
    actions = std::vector<float>(outputCount);
    std::fill(std::begin(actions), std::end(actions), 0.f);

    constructGenome(inputCount, outputCount);

    MarkovNames nameGenerator(false, world->getConfig().seed++);
    std::vector<double> genome;
    genes->writeNormal(genome);
    name = nameGenerator.generate(genome);
    color = colorFromGenome(genome);
    sprite.setColor(color);

}

Agent::Agent(const Agent &other, float mutation) : WorldObject(other), orientation(other.orientation) {
    loadResources();
    generation = other.generation;
    maxEnergy = other.maxEnergy;
    energy = other.energy;
    energyLossRate = other.energyLossRate;
    maxSpeed = other.maxSpeed;
    turnFactor = other.turnFactor;

    frameIndex = 0;
    frame = sf::IntRect(0, 0, 32, 32);
    sprite = sf::Sprite(walkingTexture, frame);
    sprite.setOrigin(other.sprite.getOrigin());
    setBounds(other.getBounds());

    percept.resize(other.percept.size());
    actions.resize(other.actions.size());
    genes = std::dynamic_pointer_cast<MapGenes>(other.genes->clone());
    genes->mutate(mutation);

    MarkovNames nameGenerator(false, world->getConfig().seed++);
    std::vector<double> genome;
    std::vector<double> genome2;
    genes->writeNormal(genome);
    name = nameGenerator.generate(genome);
    other.genes->writeNormal(genome2);
    printf("%f - %f\n", std::accumulate(genome2.begin(), genome2.end(), 0.0), std::accumulate(genome.begin(), genome.end(), 0.0));
    color = colorFromGenome(genome);
    sprite.setColor(color);

    // Vision variables
    receptorCount = other.receptorCount;
    visibilityDistance = other.visibilityDistance;
    FOV = other.FOV;
    visualReactivity = other.visualReactivity;

    receptors.resize(receptorCount);
    std::fill(std::begin(receptors), std::end(receptors), 0.f);

    lineOfSight.resize(receptorCount*2);
    lineOfSight[0] = sf::Vertex(sf::Vector2f(0,0));
    lineOfSight[1] = sf::Vertex(sf::Vector2f(0,0));
    lineOfSight[0].color = sf::Color::Cyan;
    lineOfSight[1].color = sf::Color::Cyan;

    sf::Vertex orientationLine[2];
}


void Agent::constructGenome(size_t inputCount, size_t outputCount) {
    auto previousLayerPerceptronCountLambda = [](LambdaGene<int> &l, float mutationFactor) {
        auto layers = l.getOwner<MapGenes>()->getOwner<ListGenes>()->getOwner<MapGenes>()->getOwner<ListGenes>();
        auto thisLayer = l.getOwner<MapGenes>()->getOwner<ListGenes>()->getOwner<MapGenes>();

        auto itr = layers->getList().begin();
        for (auto &_ : layers->getList()) {
            if (itr->get() == thisLayer){
                break;
            }
            itr++;
        }

        if (itr == layers->getList().begin()){
            auto count = layers->getOwner<MapGenes>()->getGene<IntegerGene>("InputCount");
            count->evaluate(mutationFactor, l.getEvaluationCount());
            return count->getValue();
        }

        itr--;
        auto lastLayer = ((MapGenes*) itr->get());
        auto count = lastLayer->getGene<LambdaGene<int> >("PerceptronCount");
        count->evaluate(mutationFactor, l.getEvaluationCount());
        return count->getValue();

    };

    auto perceptronCountLambda = [](LambdaGene<int> &l, float mutationFactor) {
        auto layers = l.getOwner<MapGenes>()->getOwner<ListGenes>();
        auto thisLayer = l.getOwner<MapGenes>();

        auto itr = layers->getList().begin();
        for (auto &_ : layers->getList()) {
            if (itr->get() == thisLayer){
                break;
            }
            itr++;
        }

        if (++itr == layers->getList().end()){
            auto count = layers->getOwner<MapGenes>()->getGene<IntegerGene>("OutputCount");
            count->evaluate(mutationFactor, l.getEvaluationCount());
            return count->getValue();
        }

        auto count = l.getOwner<MapGenes>()->getGene<IntegerGene>("MutatingPerceptronCount");
        count->evaluate(mutationFactor, l.getEvaluationCount());
        return count->getValue();
    };

    auto& aConfig = world->getConfig().agents;

    auto perceptron = std::make_shared<MapGenes>();
    auto weightCount = std::make_shared<LambdaGene<int> >(previousLayerPerceptronCountLambda);
    perceptron->addGenes("WeightCount", weightCount);
    auto weight = std::make_shared<FloatGene>(aConfig.weightMin, aConfig.weightMax);
    auto weights = std::make_shared<ListGenes>(weight, "WeightCount");
    perceptron->addGenes("Weights", weights);

    auto layer = std::make_shared<MapGenes>();
    auto mutatingPerceptronCount = std::make_shared<IntegerGene>(aConfig.perceptronPerLayerMin, aConfig.perceptronPerLayerMax);
    layer->addGenes("MutatingPerceptronCount", mutatingPerceptronCount);
    auto perceptronCount = std::make_shared<LambdaGene<int> >(perceptronCountLambda);
    layer->addGenes("PerceptronCount", perceptronCount);
    auto bias = std::make_shared<FloatGene>(aConfig.biasMin, aConfig.biasMax);
    layer->addGenes("Bias", bias);
    auto perceptrons = std::make_shared<ListGenes>(perceptron, "PerceptronCount");
    layer->addGenes("Perceptrons", perceptrons);

    genes = std::make_shared<MapGenes>();
    auto layerCount = std::make_shared<IntegerGene>(aConfig.layersMin, aConfig.layersMax);
    genes->addGenes("LayerCount", layerCount);
    auto inputCountG = std::make_shared<IntegerGene>(inputCount, inputCount);
    genes->addGenes("InputCount", inputCountG);
    auto outputCountG = std::make_shared<IntegerGene>(outputCount, outputCount);
    genes->addGenes("OutputCount", outputCountG);
    auto layers = std::make_shared<ListGenes>(layer, "LayerCount");
    genes->addGenes("Layers", layers);

    genes->generate();
}

void Agent::update(float deltaTime) {
    WorldObject::update(deltaTime);

    // Apply actions
    sf::Vector2f orientationVector = {
            cosf(orientation*PI/180.f),
            sinf(orientation*PI/180.f)
    };

    applyForce(deltaTime, orientationVector * actions.at(0) * maxSpeed);

    float turn = ((float) actions.at(1) - actions.at(2))*turnFactor;
    orientation += turn*deltaTime;

    if (0.7 < actions.at(3) && 80 < energy){
        world->reproduce(*this);
    }

    if (quadtree != nullptr){
        auto near = quadtree->searchNear(getPosition(), 64);
        for (auto &object : near) {
            if (object.get() != this ){
                sf::FloatRect a(getPosition().x + getBounds().left,
                                getPosition().y + getBounds().top,
                                getBounds().width - getBounds().left,
                                getBounds().height - getBounds().top);

                sf::FloatRect b(object->getPosition().x + object->getBounds().left,
                                object->getPosition().y + object->getBounds().top,
                                object->getBounds().width - object->getBounds().left,
                                object->getBounds().height - object->getBounds().top);
                if (boxesIntersect(a, b)){
                    auto &type = typeid(*object.get());
                    if (type == typeid(Agent)){
                        if (0.5 < actions.at(3)){
                            // TODO: Crossover reproduction.
                        }
                    }
                    else if (type == typeid(Mushroom)){
                        world->removeObject(object->getSharedPtr(), false);
                        energy += world->getConfig().agents.mushroomEnergy;
                    }
                }
            }
        }
    }

    energy = fminf(energy, 100);
    energy -= deltaTime*energyLossRate;
    if (energy <= 0){
        world->removeObject(getSharedPtr(), false);
    }
}

void Agent::draw(sf::RenderWindow *window, float deltaTime) {
    frameTimer += deltaTime;
    if (actions.at(0)*100.f < frameTimer*1000.f){
        frameIndex = frameIndex % 12 + 1; // Skip the first frame
        frame.top = frameIndex * 32;
        sprite.setTextureRect(frame);
        int o = ((360 + (int) orientation%360 + 315))%360 / 90;
        if (o == 1) o = 3; // To match the order in the image
        else if (o == 3) o = 1;

        frame.left = o*32;
        frameTimer = 0;
    }

    sprite.setPosition(getPosition());
    window->draw(sprite);

    if (world->getConfig().render.showVision){
        window->draw(&lineOfSight.front(), 2*receptors.size(), sf::Lines);
        window->draw(orientationLine, 2, sf::Lines);
    }
    WorldObject::draw(window, deltaTime);
}

void Agent::updatePercept(float deltaTime) {
    auto perceptIterator = percept.begin();

    // Calculate perceptors

    sf::Vector2f visionEnd = getPosition() + sf::Vector2f(visibilityDistance, 0);
    sf::Vector2f dV = visionEnd - getPosition();

    for (size_t i = 0; i < receptors.size(); i++){
        float angle = (orientation - FOV/2.f + FOV*((float) i/(receptors.size()-1)))*PI/180.f;
        sf::Vector2f lineEnd = {
                dV.x * cosf(angle) - dV.y * sinf(angle),
                dV.x * sinf(angle) - dV.y * cosf(angle)
        };

        std::vector<std::shared_ptr<WorldObject> > nl;
        quadtree->searchNearLine(nl, getPosition(), getPosition()+lineEnd);

        for (auto &n : nl){
            if (n.get() != this && typeid(*n.get()) == typeid(Mushroom)){
                sf::Vector2f a = sf::Vector2f(n->getPosition().x + n->getBounds().left,
                                              n->getPosition().y + n->getBounds().top);

                sf::Vector2f b = sf::Vector2f(n->getBounds().width - n->getBounds().left,
                                              n->getBounds().height - n->getBounds().top);

                if (lineIntersectWithBox(getPosition(), getPosition()+lineEnd, a, b)){
                    sf::Vector2f dPos = n->getPosition() - getPosition();
                    float change = deltaTime*visualReactivity*(1.f-(dPos.x*dPos.x+dPos.y*dPos.y)/(visibilityDistance*visibilityDistance));
                    receptors[i] = (1-deltaTime*change)*receptors[i] + change;
                }
            }
        }

        receptors[i] = (1-deltaTime*visualReactivity)*receptors[i] + 0;
        *perceptIterator = receptors[i];
        perceptIterator++;

        if (world->getConfig().render.showVision) {
            lineOfSight[i*2].position = getPosition();
            lineOfSight[i*2+1].position = getPosition() + lineEnd;
            lineOfSight[i*2].color = sf::Color(155*receptors[i]+100, 10, 10, 255);
            lineOfSight[i*2+1].color = lineOfSight[i*2].color;
        }

    }

    if (world->getConfig().render.showVision){
        sf::Vector2f lineEnd = {
                dV.x * cosf(orientation*PI/180.f) - dV.y * sinf(orientation*PI/180.f),
                dV.x * sinf(orientation*PI/180.f) - dV.y * cosf(orientation*PI/180.f)
        };

        orientationLine[0].position = getPosition();
        orientationLine[1].position = getPosition() + lineEnd;
        orientationLine[0].color = sf::Color(100, 100, 200);
        orientationLine[1].color = orientationLine[0].color;
    }

    *perceptIterator = energy / maxEnergy;
    perceptIterator++;

    if (perceptIterator != percept.end()){
        throw std::runtime_error("All percept values not updated.\n");
    }
}

MapGenes *Agent::getGenes() const {
    return genes.get();
}

float Agent::getOrientation() const {
    return orientation;
}

void Agent::setOrientation(float orientation) {
    Agent::orientation = orientation;
}

const std::vector<float> &Agent::getPercept() const {
    return percept;
}

void Agent::setPercept(const std::vector<float> &percept) {
    Agent::percept = percept;
}

const std::vector<float> &Agent::getActions() const {
    return actions;
}

void Agent::setActions(const std::vector<float> &action) {
    Agent::actions = action;
}

float Agent::getEnergy() const {
    return energy;
}

void Agent::setEnergy(float energy) {
    Agent::energy = energy;
}

float Agent::getMaxEnergy() const {
    return maxEnergy;
}

void Agent::setMaxEnergy(float maxEnergy) {
    Agent::maxEnergy = maxEnergy;
}

const sf::Color &Agent::getColor() const {
    return color;
}

const std::string &Agent::getName() const {
    return name;
}

unsigned int Agent::getGeneration() const {
    return generation;
}

void Agent::setGeneration(unsigned int generation) {
    Agent::generation = generation;
}

