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
#include "Skull.h"

#define PI 3.14159265f

bool Agent::loaded = false;
sf::Texture Agent::walkingTexture;
sf::Texture Agent::punchTexture;

void Agent::loadResources() {
    if (!loaded){
        Agent::walkingTexture.loadFromFile("resources/WalkCycle.png");
        Agent::punchTexture.loadFromFile("resources/Punch.png");
        loaded = true;
    }
}

Agent::Agent(const AgentSettings &settings, World *world, sf::Vector2f position, float orientation)
: settings(settings), WorldObject("Agent", world, position, true), orientation(orientation) {
    loadResources();

    generation = 0;
    childCount = 0;
    energy = settings.maxEnergy;
    setMass(settings.mass);
    setFriction(settings.friction);
    actionCooldown = 0;
    punchTimer = 0;

    inventory.mushrooms = 0;

    frameIndex = 0;
    frame = sf::IntRect(0, 0, 32, 32);
    sprite = sf::Sprite(walkingTexture, frame);
    sprite.setOrigin(16, 5);
    setBounds(sf::IntRect(-8, 10, 8, 27));

    receptors.resize(settings.receptorCount);
    std::fill(std::begin(receptors), std::end(receptors), 0.f);

    lineOfSight.resize(settings.receptorCount*2);
    lineOfSight[0] = sf::Vertex(sf::Vector2f(0,0));
    lineOfSight[1] = sf::Vertex(sf::Vector2f(0,0));
    lineOfSight[0].color = sf::Color::Cyan;
    lineOfSight[1].color = sf::Color::Cyan;

    std::size_t inputCount = settings.perceiveCollision + receptors.size() + 3*settings.perceiveColor
            + settings.perceiveEnergyLevel  + settings.memory + settings.perceiveMushroomCount;
    std::size_t outputCount = settings.canReproduce + settings.canWalk + 2*settings.canTurn + settings.canEat + settings.canPlace + settings.canPunch + settings.memory;

    percept = std::vector<float>(inputCount);
    std::fill(std::begin(percept), std::end(percept), 0.f);
    actions = std::vector<float>(outputCount);
    std::fill(std::begin(actions), std::end(actions), 0.f);
    memory = std::vector<float>(settings.memory);
    std::fill(std::begin(memory), std::end(memory), 0.f);

    constructGenome(inputCount, outputCount);

    MarkovNames nameGenerator(false, world->getConfig().seed++);
    std::vector<double> genome;
    genes->writeNormal(genome);
    name = nameGenerator.generate(genome);
    setColor(colorFromGenome(genome));
    sprite.setColor(color);

}

Agent::Agent(const Agent &other, float mutation)
: settings(other.settings), WorldObject(other), orientation(other.orientation) {
    loadResources();
    generation = other.generation;
    childCount = 0;
    energy = other.energy;
    actionCooldown = 0;
    punchTimer = 0;

    inventory.mushrooms = 0;

    frameIndex = 0;
    frame = sf::IntRect(0, 0, 32, 32);
    sprite = sf::Sprite(walkingTexture, frame);
    sprite.setOrigin(other.sprite.getOrigin());
    setBounds(other.getBounds());

    actions.resize(other.actions.size());
    percept.resize(other.percept.size());
    memory.resize(other.memory.size());
    genes = std::dynamic_pointer_cast<MapGenes>(other.genes->clone());
    genes->mutate(mutation);

    MarkovNames nameGenerator(false, world->getConfig().seed++);
    std::vector<double> genome;
    std::vector<double> genome2;
    genes->writeNormal(genome);
    name = nameGenerator.generate(genome);
    other.genes->writeNormal(genome2);
    setColor(colorFromGenome(genome));
    sprite.setColor(color);

    // AI
    receptors.resize(other.receptors.size());
    std::fill(std::begin(receptors), std::end(receptors), 0.f);

    lineOfSight.resize(settings.receptorCount*2);
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

    auto perceptron = std::make_shared<MapGenes>();
    auto weightCount = std::make_shared<LambdaGene<int> >(previousLayerPerceptronCountLambda);
    perceptron->addGenes("WeightCount", weightCount);
    auto weight = std::make_shared<FloatGene>(settings.weightMin, settings.weightMax);
    auto weights = std::make_shared<ListGenes>(weight, "WeightCount");
    perceptron->addGenes("Weights", weights);

    auto layer = std::make_shared<MapGenes>();
    auto mutatingPerceptronCount = std::make_shared<IntegerGene>(settings.perceptronPerLayerMin, settings.perceptronPerLayerMax);
    layer->addGenes("MutatingPerceptronCount", mutatingPerceptronCount);
    auto perceptronCount = std::make_shared<LambdaGene<int> >(perceptronCountLambda);
    layer->addGenes("PerceptronCount", perceptronCount);
    auto bias = std::make_shared<FloatGene>(settings.biasMin, settings.biasMax);
    layer->addGenes("Bias", bias);
    auto perceptrons = std::make_shared<ListGenes>(perceptron, "PerceptronCount");
    layer->addGenes("Perceptrons", perceptrons);

    genes = std::make_shared<MapGenes>();
    auto layerCount = std::make_shared<IntegerGene>(settings.layersMin, settings.layersMax);
    genes->addGenes("LayerCount", layerCount);
    auto inputCountG = std::make_shared<IntegerGene>(inputCount, inputCount);
    genes->addGenes("InputCount", inputCountG);
    auto outputCountG = std::make_shared<IntegerGene>(outputCount, outputCount);
    genes->addGenes("OutputCount", outputCountG);
    auto layers = std::make_shared<ListGenes>(layer, "LayerCount");
    genes->addGenes("Layers", layers);

    genes->generate();
}

void Agent::updatePercept(float deltaTime) {
    auto perceptIterator = percept.begin();

    if (settings.perceiveCollision){
        *perceptIterator = isColliding();
        perceptIterator++;
    }

    // Calculate perceptors

    sf::Vector2f visionEnd = getPosition() + sf::Vector2f(settings.visibilityDistance, 0);
    sf::Vector2f dV = visionEnd - getPosition();
    unsigned averageColor[3] = {0, 0, 0};
    unsigned objectsSeen = 0;

    for (size_t i = 0; i < receptors.size(); i++){
        float angle = (orientation - settings.FOV/2.f + settings.FOV*((float) i/(receptors.size()-1)))*PI/180.f;
        sf::Vector2f lineEnd = {
                dV.x * cosf(angle) - dV.y * sinf(angle),
                dV.x * sinf(angle) - dV.y * cosf(angle)
        };

        std::vector<std::shared_ptr<WorldObject> > nl;
        quadtree->searchNearLine(nl, getPosition(), getPosition()+lineEnd);

        for (auto &n : nl){
            if (n.get() != this){
                sf::Vector2f a = sf::Vector2f(n->getPosition().x + n->getBounds().left,
                                              n->getPosition().y + n->getBounds().top);

                sf::Vector2f b = sf::Vector2f(n->getBounds().width - n->getBounds().left,
                                              n->getBounds().height - n->getBounds().top);

                if (lineIntersectWithBox(getPosition(), getPosition()+lineEnd, a, b)){
                    const auto col = n->getColor();
                    objectsSeen++;
                    averageColor[0] += col.r; averageColor[1] += col.b; averageColor[2] += col.g;
                    sf::Vector2f dPos = n->getPosition() - getPosition();
                    float change = deltaTime*settings.visualReactivity*(1.f-(dPos.x*dPos.x+dPos.y*dPos.y)/(settings.visibilityDistance*settings.visibilityDistance));
                    if (typeid(*n) == typeid(Agent)){
                        change /= 3.f;
                    }
                    receptors[i] = (1-deltaTime*change)*receptors[i] + change;
                }
            }
        }

        receptors[i] = (1-deltaTime*settings.visualReactivity)*receptors[i] + 0;
        *perceptIterator = receptors[i];
        perceptIterator++;

        if (world->getConfig().render.showVision) {
            lineOfSight[i*2].position = getPosition();
            lineOfSight[i*2+1].position = getPosition() + lineEnd;
            lineOfSight[i*2].color = sf::Color(155*receptors[i]+100, 155*receptors[i]+100, 155*receptors[i]+100, 255);
            lineOfSight[i*2+1].color = lineOfSight[i*2].color;
        }

    }

    if (settings.perceiveColor) {
        sf::Color averageCol(125, 125, 125);
        if (objectsSeen != (unsigned) 0){
            averageCol = sf::Color(averageColor[0]/objectsSeen, averageColor[1]/objectsSeen, averageColor[2]/objectsSeen);
        }
        if (world->getConfig().render.showVision){
            for (auto& los : lineOfSight){
                los.color *= averageCol;
            }
        }

        *perceptIterator = averageCol.r/255.f;
        perceptIterator++;

        *perceptIterator = averageCol.b/255.f;;
        perceptIterator++;

        *perceptIterator = averageCol.g/255.f;
        perceptIterator++;

    }

    if (world->getConfig().render.showVision){
        sf::Vector2f lineEnd = {
                dV.x * cosf(orientation*PI/180.f) - dV.y * sinf(orientation*PI/180.f),
                dV.x * sinf(orientation*PI/180.f) - dV.y * cosf(orientation*PI/180.f)
        };

        orientationLine[0].position = getPosition();
        orientationLine[1].position = getPosition() + lineEnd;
        orientationLine[0].color = sf::Color(100, 100, 200, 50);
        orientationLine[1].color = orientationLine[0].color;
    }


    if (settings.perceiveEnergyLevel){
        *perceptIterator = energy / settings.maxEnergy;
        perceptIterator++;
    }

    if (settings.perceiveMushroomCount){
        *perceptIterator = inventory.mushrooms / settings.maxMushroomCount;
        perceptIterator++;
    }

    for (auto& mem : memory){
        *perceptIterator = mem;
        perceptIterator++;
    }

    if (perceptIterator != percept.end()){
        throw std::runtime_error("All percept values not updated. At "
                                 +std::to_string(perceptIterator - percept.begin())+" of "+std::to_string(percept.size()));
    }
}

void Agent::update(float deltaTime) {
    WorldObject::update(deltaTime);
    actionCooldown = fmaxf(actionCooldown - deltaTime, 0.f);

    // Apply actions
    auto actionIterator = actions.begin();

    sf::Vector2f orientationVector = {
            cosf(orientation*PI/180.f),
            sinf(orientation*PI/180.f)
    };
    if (settings.canWalk){
        float walk = *(actionIterator++)-0.3f;
        if (punchTimer == 0){
            applyForce(deltaTime, orientationVector * walk * settings.maxSpeed);
            energy -= fabsf(walk) * settings.movementEnergyLoss * deltaTime;
        }
    }

    if (settings.canTurn){
        const float turn1 = *(actionIterator++);
        const float turn2 = *(actionIterator++);
        float turn = (turn1 - turn2)*settings.turnFactor;
        orientation += turn*deltaTime;
    }



    if (settings.canReproduce){
        const float reproduceWilling = *(actionIterator++);
        if (0.6 < reproduceWilling && 80 < energy && actionCooldown == 0){
            actionCooldown = settings.actionCooldown;
            world->reproduce(*this);
        }
    }


    if (settings.canEat){
        const float eatWilling = *(actionIterator++) + 1;
        if (0 < inventory.mushrooms && 0.7 < eatWilling && actionCooldown == 0){
            actionCooldown = settings.actionCooldown;
            inventory.mushrooms--;
            energy += world->getConfig().agents.mushroomEnergy;
        }
    }

    if (settings.canPlace){
        const float placeWilling = *(actionIterator++);
        if (0 < inventory.mushrooms && 0.7 < placeWilling && actionCooldown == 0) {
            actionCooldown = settings.actionCooldown;
            inventory.mushrooms--;
            sf::Vector2<float> position(getPosition().x+50*orientationVector.x, getPosition().y+50*orientationVector.y);
            std::shared_ptr<Mushroom> mushroom(new Mushroom(world, position));
            world->addObject(mushroom);
        }
    }

    if (settings.canPunch){
        const float punchWilling = *(actionIterator++);
        if (punchTimer == 0 && 0.7 < punchWilling){
            punchTimer += deltaTime;
            sprite.setTexture(punchTexture);
            energy -= settings.punchEnergy;

        }
        else if (settings.punchTime < punchTimer){
            punchTimer = 0;
            frameIndex = 0;
            frame = sf::IntRect(0, 0, 32, 32);
            sprite.setTexture(walkingTexture);
        }
        else if (punchTimer != 0) {
            punchTimer += deltaTime;
        }
    }


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
                    if (punchTimer == deltaTime){
                        auto enemy = (Agent*) object.get();
                        enemy->setEnergy(enemy->getEnergy() - settings.punchDamage);
                        if (enemy->getEnergy() < 0){
                            printf("Agent %s murdered %s\n", name.c_str(), enemy->name.c_str());
                            inventory.mushrooms += enemy->inventory.mushrooms;
                            enemy->inventory.mushrooms = 0;
                            world->addObject(std::make_shared<Skull>(world, enemy->getPosition()));
                        }
                    }
                }
                else if (type == typeid(Mushroom) && inventory.mushrooms < settings.maxMushroomCount){
                    world->removeObject(object->getSharedPtr(), false);
                    inventory.mushrooms++;
                }
            }
        }
    }

    energy = fminf(energy, 100);
    energy -= deltaTime*settings.energyLossRate;
    if (energy <= 0){
        world->removeObject(getSharedPtr(), false);
    }

    for (float &mem : memory) {
        mem = *(actionIterator++);
    }

    if (actionIterator != actions.end()){
        throw std::runtime_error("All actions values not accessed. At "
                                 +std::to_string(actionIterator - actions.begin())+" of "+std::to_string(actions.size()));
    }
}

void Agent::draw(sf::RenderWindow *window, float deltaTime) {
    frameTimer += deltaTime;
    if (actions.at(0)*100.f < frameTimer*1000.f && punchTimer == 0){
        frameIndex = frameIndex % 12 + 1; // Skip the first frame
        frame.top = frameIndex * 32;
        sprite.setTextureRect(frame);
        int o = ((360 + (int) orientation%360 + 315))%360 / 90;
        if (o == 1) o = 3; // To match the order in the image
        else if (o == 3) o = 1;

        frame.left = o*32;
        frameTimer = 0;
    }
    else if (settings.punchTime / 5.f < frameTimer){
        frameIndex = (frameIndex + 1) % 5;
        frame.top = frameIndex*32;
        int o = ((360 + (int) orientation%360 + 315))%360 / 90;
        if (o == 1) o = 3; // To match the order in the image
        else if (o == 3) o = 1;
        frame.left = o*32;
        sprite.setTextureRect(frame);
        frameTimer = 0;
    }

    sprite.setPosition(getPosition());
    if (world->getConfig().render.renderGeneration){
        unsigned deltaGeneration = world->getStatistics().highestGeneration - world->getStatistics().lowestGeneration;
        if (deltaGeneration != 0){
            sf::Color c = sprite.getColor();
            c.a = 200 * (float) (generation - world->getStatistics().lowestGeneration) / (float) deltaGeneration + 55;
            sprite.setColor(c);
        }
    }
    else {
        sf::Color c = sprite.getColor();
        c.a = 255;
        sprite.setColor(c);
    }
    window->draw(sprite);

    if (world->getConfig().render.showVision){
        window->draw(&lineOfSight.front(), 2*receptors.size(), sf::Lines);
        window->draw(orientationLine, 2, sf::Lines);
    }
    WorldObject::draw(window, deltaTime);
}

const AgentSettings &Agent::getSettings() const {
    return settings;
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

void Agent::setActions(const std::vector<float> &actions) {
    Agent::actions = actions;
}

float Agent::getEnergy() const {
    return energy;
}

void Agent::setEnergy(float energy) {
    Agent::energy = energy;
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

unsigned int Agent::getChildCount() const {
    return childCount;
}

void Agent::setChildCount(unsigned int childCount) {
    Agent::childCount = childCount;
}

const Agent::Inventory &Agent::getInventory() const {
    return inventory;
}

void Agent::setInventory(const Agent::Inventory &inventory) {
    Agent::inventory = inventory;
}


