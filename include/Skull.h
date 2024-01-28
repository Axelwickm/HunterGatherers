//
// Created by Axel on 2019-01-15.
//

#ifndef HUNTERGATHERERS_SKULL_H
#define HUNTERGATHERERS_SKULL_H


#include "WorldObject.h"

class Skull : public WorldObject {
public:
    Skull(World *world, const sf::Vector2f &position);
    static void loadResources();

    void update(float deltaTime) override;
    void draw(sf::RenderWindow *window, float deltaTime) override;

private:
    sf::Sprite sprite;

    static bool loaded;
    static sf::Texture texture;

};


#endif //HUNTERGATHERERS_SKULL_H
