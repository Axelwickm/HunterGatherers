//
// Created by axelw on 2018-12-28.
//

#include "Mushroom.h"
#include "World.h"

bool Mushroom::loaded = false;
sf::Texture Mushroom::texture;
std::mt19937 Mushroom::randomEngine = std::mt19937(0); // Seed is set outside class

void Mushroom::loadResources() {
    if (!loaded){
        loaded = true;
        texture.loadFromFile("resources/Mushroom.png");
    }
}

Mushroom::Mushroom(World *world, const sf::Vector2f &position, const Config &config)
: WorldObject("Mushroom", world, position, true), config(config) {
    dist = std::uniform_real_distribution<float>(0, 1);
    loadResources();
    sprite = sf::Sprite(texture);
    sprite.setScale(0.5f, 0.5f);
    sf::FloatRect localBounds = sprite.getLocalBounds();
    WorldObject::setBounds(sf::IntRect(localBounds.left*0.5f, localBounds.top*0.5f,
            localBounds.width*0.5f, localBounds.height*0.5f));
    WorldObject::setColor(sf::Color::Red);
}

void Mushroom::update(float deltaTime) {
    WorldObject::update(deltaTime);

    auto popEntry = world->getPopulator().getEntry("Mushroom");
    float rand = dist(randomEngine);
    if (rand < config.world.mushroomReproductionRate*deltaTime && popEntry.count < popEntry.targetCount) {
        auto near = world->getQuadtree().searchNear(position, config.world.mushroomReproductionDistance);
        unsigned count = 0;
        for (auto &e : near){
            if (typeid(*e.get()) == typeid(Mushroom)){
                auto diff = getPosition() - e->getPosition();
                float d = std::sqrt(diff.x*diff.x+diff.y*diff.y);
                if (d < config.world.mushroomReproductionDistance)
                    count++;
            }
        }

        float delta = std::max(0, int(config.world.mushroomReproductionNearLimit-count))
                /(float) config.world.mushroomReproductionNearLimit;

        if (rand < config.world.mushroomReproductionRate*deltaTime*delta){
            auto angleDist = std::uniform_real_distribution<double>(0, 360);
            double angle = angleDist(randomEngine);
            sf::Vector2f pos = position+sf::Vector2f(
                    std::sin(angle) * config.world.mushroomReproductionDistance,
                    std::cos(angle) * config.world.mushroomReproductionDistance);

            auto mushroom = std::make_shared<Mushroom>(world, pos, config);
            world->addObject(mushroom);
        }
    }
}

void Mushroom::draw(sf::RenderWindow *window, float deltaTime) {
    sprite.setPosition(getPosition());
    window->draw(sprite);
    WorldObject::draw(window, deltaTime);
}
