//
// Created by axelw on 2018-12-28.
//

#ifndef HUNTERGATHERERS_MUSHROOM_H
#define HUNTERGATHERERS_MUSHROOM_H


#include "WorldObject.h"

class Mushroom : public WorldObject {
public:
    Mushroom(World *world, const sf::Vector2f &position);
    static void loadResources();

    void draw(sf::RenderWindow *window, float deltaTime) override;
private:
    sf::Sprite sprite;

    static bool loaded;
    static sf::Texture texture;
};


#endif //HUNTERGATHERERS_MUSHROOM_H
