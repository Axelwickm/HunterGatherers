//
// Created by axelw on 2019-01-05.
//

#include "Heart.h"
#include "World.h"

bool Heart::loaded = false;
sf::Texture Heart::texture;

void Heart::loadResources() {
    if (!loaded){
        loaded = true;
        texture.loadFromFile("resources/Heart.png");
    }
}

Heart::Heart(World *world, const sf::Vector2f &position)
: WorldObject("Heart", world, position, false) {
    loadResources();
    sprite = sf::Sprite(texture);
    setMass(0.5);
    setVelocity(sf::Vector2f(0, -20));
}

void Heart::update(float deltaTime) {
    WorldObject::update(deltaTime);
    if (1 < getAge()){
        world->removeObject(getSharedPtr(), false);
    }
}

void Heart::draw(sf::RenderWindow *window, float deltaTime) {
    sprite.setPosition(getPosition());
    window->draw(sprite);
    WorldObject::draw(window, deltaTime);
}


