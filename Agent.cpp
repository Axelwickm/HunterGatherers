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

Agent::Agent(World* world, sf::Vector2f position) : WorldObject(world, position) {
    orientation = 75;
    setAccelerationFactor(0.25);

    r.setSize(sf::Vector2f(10, 10));
    r.setOrigin(5, 5);
    r.setFillColor(sf::Color::Red);

    visibility = 200;
    FOV = 110;
    visualReactivity = 3;

    receptors.resize(acuity);
    std::fill(std::begin(receptors), std::end(receptors), 0.f);

    lineOfVision[0] = sf::Vertex(sf::Vector2f(0,0));
    lineOfVision[1] = sf::Vector2f(0,0);
    lineOfVision[0].color = sf::Color::Cyan;
    lineOfVision[1].color = sf::Color::Cyan;

    neuralNet.maxLayerSize = std::max(*std::max_element(std::begin(neuralNet.layerSizes), std::end(neuralNet.layerSizes)),
            neuralNet.inputBandwidth);

    percept = std::vector<float>(neuralNet.maxLayerSize);
    std::fill(std::begin(percept), std::end(percept), 0.f);
    actions = std::vector<float>(neuralNet.outputBandwidth);
    std::fill(std::begin(actions), std::end(actions), 0.f);

    MapGenes genes;
    std::shared_ptr<FloatGene> originalFloat = std::make_shared<FloatGene>(0, 1);
    std::shared_ptr<MapGenes> innerMap = std::make_shared<MapGenes>();

    auto lambda = [](LambdaGene<float>& l, float mutationFactor){
        FloatGene* g = (FloatGene*) ((MapGenes*) l.getOwner()->getOwner())->getGene("originalFloat");
        g->evaluate(mutationFactor, l.getEvaluationCount());
        return g->getValue()*2.f;
    };

    std::shared_ptr<Gene> l = std::make_shared<LambdaGene<float>>(lambda);
    innerMap->addGenes("derivedValue", l);
    genes.addGenes("originalFloat", originalFloat);
    genes.addGenes("innerMap", innerMap);
    auto integerGene = std::make_shared<IntegerGene>(0, 10);
    auto templateGene = std::make_shared<IntegerGene>(0, 2);
    genes.addGenes("int",  integerGene);
    innerMap->addGenes("dependentList", std::make_shared<ListGenes>(templateGene, integerGene));

    genes.generate();

    printf("\n-----\n");

    printf("Original: %f\n", (genes.getGene<FloatGene>("originalFloat"))->getValue());
    printf("New: %f\n", genes.getGene<MapGenes>("innerMap")->getGene<LambdaGene<float>>("derivedValue")->getValue());
    printf("Int: %d\n", genes.getGene<IntegerGene>("int")->getValue());

    printf("\n-----\nMutation:\n");
    genes.mutate(0.25);

    printf("\n-----\n");

    printf("Original: %f\n", ((FloatGene*) genes.getGene("originalFloat"))->getValue());
    printf("New: %f\n", ((LambdaGene<float>*)((MapGenes*) genes.getGene("innerMap"))->getGene("derivedValue"))->getValue());
    printf("Int: %d\n", genes.getGene<IntegerGene>("int")->getValue());

    printf("\n-----\n");

    printf("Cloning\n");
    std::shared_ptr<MapGenes> a2 = std::static_pointer_cast<MapGenes>(genes.Clone());
    printf("Mutate 1\n");
    genes.mutate(0);
}

void Agent::update(float deltaTime) {
    WorldObject::update(deltaTime);
    // Apply actions
    sf::Vector2f vel = getVelocity();
    float velocityFactor = (actions.at(0)*20.f / sqrtf(vel.x*vel.x+vel.y*vel.y))*10.f*deltaTime;
    sf::Vector2f orientationVector = {
            cosf(orientation*PI/180.f),
            sinf(orientation*PI/180.f)
    };
    setVelocity(getVelocity() + orientationVector * velocityFactor);

    float turn = ((float) actions.at(1) - actions.at(2))*10.f*deltaTime;
    orientation += 0.5f < fabs(turn) ? turn : 0;
}

void Agent::draw(sf::RenderWindow *window, float deltaTime) {
    r.setPosition(getPosition());
    window->draw(r);

    if (RenderSettings::showVision){
        window->draw(lineOfVision, 2*receptors.size(), sf::Lines);
    }
}

void Agent::updatePercept(float deltaTime) {

    // Calculate perceptors

    sf::Vector2f visionEnd = getPosition() + sf::Vector2f(visibility, 0);
    sf::Vector2f dV = visionEnd - getPosition();

    for (size_t i = 0; i < receptors.size(); i++){
        float angle = ((float) orientation - FOV/2.f + FOV*((float) i/receptors.size()))*PI/180.f;
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
}

const NeuralNet &Agent::getNeuralNet() const {
    return neuralNet;
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

