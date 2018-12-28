//
// Created by axelw on 2018-12-28.
//

#include "Mushroom.h"

bool Mushroom::loaded = false;
sf::Texture Mushroom::texture;

void Mushroom::loadResources() {
    if (!loaded){
        loaded = true;
        texture.loadFromFile("resources/Mushroom.png");
    }
}

Mushroom::Mushroom(World *world, const sf::Vector2f &position) : WorldObject(world, position) {
    loadResources();
    sprite = sf::Sprite(texture);
}

void Mushroom::draw(sf::RenderWindow *window, float deltaTime) {
    sprite.setPosition(getPosition());
    window->draw(sprite);
}


