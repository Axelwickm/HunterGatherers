//
// Created by axelw on 2018-12-28.
//

#ifndef HUNTERGATHERERS_MUSHROOM_H
#define HUNTERGATHERERS_MUSHROOM_H

#include <random>

#include "WorldObject.h"
#include "Config.h"

class Mushroom : public WorldObject {
public:
    Mushroom(World *world, const sf::Vector2f &position, const Config &config);
    static void loadResources();

    void update(float deltaTime) override;
    void draw(sf::RenderWindow *window, float deltaTime) override;

private:
    sf::Sprite sprite;

    static bool loaded;
    static sf::Texture texture;

    const Config &config;
    std::uniform_real_distribution<float> dist;
    static std::mt19937 randomEngine;
};


#endif //HUNTERGATHERERS_MUSHROOM_H
