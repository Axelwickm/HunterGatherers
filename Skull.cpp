//
// Created by Axel on 2019-01-15.
//

#include "Skull.h"
#include "World.h"

bool Skull::loaded = false;
sf::Texture Skull::texture;

void Skull::loadResources() {
    if (!loaded){
        loaded = true;
        texture.loadFromFile("resources/Skull.png");
    }
}

Skull::Skull(World *world, const sf::Vector2f &position)
        : WorldObject("Skull", world, position, false) {
    loadResources();
    sprite = sf::Sprite(texture);
    setMass(0.5);
    setVelocity(sf::Vector2f(0, -40));
}

void Skull::update(float deltaTime) {
    WorldObject::update(deltaTime);
    if (2 < getAge()){
        world->removeObject(getSharedPtr(), false);
    }
}

void Skull::draw(sf::RenderWindow *window, float deltaTime) {
    sprite.setPosition(getPosition());
    window->draw(sprite);
    WorldObject::draw(window, deltaTime);
}


