//
// Created by Axel on 2018-11-21.
//

#include <iostream>
#include <cmath>

#include "Agent.h"
#include "Config.h"
#include "utils.cpp"
#include "World.h"
#include "Gene.h"

#define PI 3.14159265f

bool Agent::loaded = false;
sf::Texture Agent::walkingTexture;

void Agent::loadResources() {
    if (!loaded){
        Agent::walkingTexture.loadFromFile("resources/WalkCycle.png");
        loaded = true;
    }
}

Agent::Agent(World* world, sf::Vector2f position) : WorldObject(world, position) {
    loadResources();

    orientation = 75;
    setAccelerationFactor(0.00000001);

    frameIndex = 0;
    frame = sf::IntRect(0, 0, 32, 32);
    sprite = sf::Sprite(walkingTexture, frame);
    sprite.setOrigin(16, 5);

    visibility = 200;
    FOV = 110;
    visualReactivity = 3;

    receptors.resize(acuity);
    std::fill(std::begin(receptors), std::end(receptors), 0.f);

    lineOfVision[0] = sf::Vertex(sf::Vector2f(0,0));
    lineOfVision[1] = sf::Vector2f(0,0);
    lineOfVision[0].color = sf::Color::Cyan;
    lineOfVision[1].color = sf::Color::Cyan;

    int inputCount = 4;
    int outputCount = 3;

    percept = std::vector<float>(4);
    std::fill(std::begin(percept), std::end(percept), 0.f);
    actions = std::vector<float>(3);
    std::fill(std::begin(actions), std::end(actions), 0.f);

    auto previousLayerPerceptronCountLambda = [](LambdaGene<int>& l, float mutationFactor){
        auto layers = l.getOwner<MapGenes>()->getOwner<ListGenes>()->getOwner<MapGenes>()->getOwner<ListGenes>();
        auto thisLayer = l.getOwner<MapGenes>()->getOwner<ListGenes>()->getOwner<MapGenes>();

        auto itr = layers->getList().begin();
        for (auto& _ : layers->getList()){
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

    auto perceptronCountLambda = [](LambdaGene<int>& l, float mutationFactor){
        auto layers = l.getOwner<MapGenes>()->getOwner<ListGenes>();
        auto thisLayer = l.getOwner<MapGenes>();

        auto itr = layers->getList().begin();
        for (auto& _ : layers->getList()){
            if (itr->get() == thisLayer){
                break;
            }
            itr++;
        }

        if (itr == layers->getList().end()){
            auto count = layers->getOwner<MapGenes>()->getGene<IntegerGene>("OutputCount");
            count->evaluate(mutationFactor, l.getEvaluationCount());
            return count->getValue();
        }
        auto count = l.getOwner<MapGenes>()->getGene<IntegerGene>("MutatingPerceptronCount");
        count->evaluate(mutationFactor, l.getEvaluationCount());
        return count->getValue();
    };

    auto perceptron = std::make_shared<MapGenes>();
    auto weightCount = std::make_shared<LambdaGene<int> >(previousLayerPerceptronCountLambda);
    perceptron->addGenes("WeightCount", weightCount);
    auto weight = std::make_shared<FloatGene>(0, 1);
    auto weights = std::make_shared<ListGenes>(weight, "WeightCount");
    perceptron->addGenes("Weights", weights);

    auto layer = std::make_shared<MapGenes>();
    auto mutatingPerceptronCount = std::make_shared<IntegerGene>(3, 20);
    layer->addGenes("MutatingPerceptronCount", mutatingPerceptronCount);
    auto perceptronCount = std::make_shared<LambdaGene<int> >(perceptronCountLambda);
    layer->addGenes("PerceptronCount", perceptronCount);
    auto bias = std::make_shared<FloatGene>(0, 1);
    layer->addGenes("Bias", bias);
    auto perceptrons = std::make_shared<ListGenes>(perceptron, "PerceptronCount");
    layer->addGenes("Perceptrons", perceptrons);

    auto layerCount = std::make_shared<IntegerGene>(2, 8);
    genes.addGenes("LayerCount", layerCount);
    auto inputCountG = std::make_shared<IntegerGene>(inputCount, inputCount);
    genes.addGenes("InputCount", inputCountG);
    auto outputCountG = std::make_shared<IntegerGene>(outputCount, outputCount);
    genes.addGenes("OutputCount", outputCountG);
    auto layers = std::make_shared<ListGenes>(layer, "LayerCount");
    genes.addGenes("Layers", layers);

    genes.generate();

}

void Agent::update(float deltaTime) {
    WorldObject::update(deltaTime);
    // Apply actions
    sf::Vector2f vel = getVelocity();
    float velocityFactor = fminf(actions.at(0)*0.75f - sqrtf(vel.x*vel.x+vel.y*vel.y)*1.f, 200.f);
    sf::Vector2f orientationVector = {
            cosf(orientation*PI/180.f),
            sinf(orientation*PI/180.f)
    };
    setVelocity(getVelocity() + orientationVector * velocityFactor * deltaTime);

    float turn = ((float) actions.at(1) - actions.at(2))*1.f*deltaTime;
    orientation += turn;
}

void Agent::draw(sf::RenderWindow *window, float deltaTime) {
    if (2500.f/actions.at(0) < frameTimer.getElapsedTime().asMilliseconds()){
        frameIndex = frameIndex % 12 + 1; // Skip the first frame
        frame.top = frameIndex * 32;
        sprite.setTextureRect(frame);
        int o = ((360 + (int) orientation%360 + 315))%360 / 90;
        if (o == 1) o = 3; // To match the order in the image
        else if (o == 3) o = 1;

        frame.left = o*32;
        frameTimer.restart();
    }

    sprite.setPosition(getPosition());
    window->draw(sprite);

    if (RenderSettings::showVision){
        window->draw(lineOfVision, 2*receptors.size(), sf::Lines);
        window->draw(orientationLine, 2, sf::Lines);
    }
}

void Agent::updatePercept(float deltaTime) {

    // Calculate perceptors

    sf::Vector2f visionEnd = getPosition() + sf::Vector2f(visibility, 0);
    sf::Vector2f dV = visionEnd - getPosition();

    for (size_t i = 0; i < receptors.size(); i++){
        float angle = (orientation - FOV/2.f + FOV*((float) i/receptors.size()))*PI/180.f;
        sf::Vector2f lineEnd = {
                dV.x * cosf(angle) - dV.y * sinf(angle),
                dV.x * sinf(angle) - dV.y * cosf(angle)
        };

        std::vector<std::shared_ptr<WorldObject> > nl;
        quadtree->searchNearLine(nl, getPosition(), getPosition()+lineEnd);

        for (auto &n : nl){
            if (n.get() != this){
                sf::Vector2f a = n->getPosition();
                sf::Vector2f b = n->getPosition() + sf::Vector2f(10, 10);
                if (lineIntersectWithBox(getPosition(), getPosition()+lineEnd, a, b)){
                    sf::Vector2f dPos = n->getPosition() - getPosition();
                    float change = deltaTime*visualReactivity*(1.f-(dPos.x*dPos.x+dPos.y*dPos.y)/(visibility*visibility));
                    receptors[i] = (1-deltaTime*change)*receptors[i] + change;
                }
            }
        }

        receptors[i] = (1-deltaTime*visualReactivity)*receptors[i] + 0;
        percept.at(i) = receptors[i];

        if (RenderSettings::showVision) {
            lineOfVision[i*2].position = getPosition();
            lineOfVision[i*2+1].position = getPosition() + lineEnd;
            lineOfVision[i*2].color =   sf::Color(245*receptors[i]+10, 245*receptors[i]+10, 245*receptors[i]+10, 255);
            lineOfVision[i*2+1].color = sf::Color(245*receptors[i]+10, 245*receptors[i]+10, 245*receptors[i]+10, 255);
        }

    }

    if (RenderSettings::showVision){
        sf::Vector2f lineEnd = {
                dV.x * cosf(orientation*PI/180.f) - dV.y * sinf(orientation*PI/180.f),
                dV.x * sinf(orientation*PI/180.f) - dV.y * cosf(orientation*PI/180.f)
        };

        orientationLine[0].position = getPosition();
        orientationLine[1].position = getPosition() + lineEnd;
        orientationLine[0].color =   sf::Color(120, 120, 200);
        orientationLine[1].color =   sf::Color(120, 120, 120);
    }
}

const MapGenes &Agent::getGenes() const {
    return genes;
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

