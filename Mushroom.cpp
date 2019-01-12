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

Mushroom::Mushroom(World *world, const sf::Vector2f &position) : WorldObject("Mushroom", world, position, true) {
    loadResources();
    sprite = sf::Sprite(texture);
    sprite.setScale(0.5f, 0.5f);
    sf::FloatRect localBounds = sprite.getLocalBounds();
    setBounds(sf::IntRect(localBounds.left*0.5f, localBounds.top*0.5f,
            localBounds.width*0.5f, localBounds.height*0.5f));
    setColor(sf::Color::Red);
}

void Mushroom::draw(sf::RenderWindow *window, float deltaTime) {
    sprite.setPosition(getPosition());
    window->draw(sprite);
    WorldObject::draw(window, deltaTime);
}


