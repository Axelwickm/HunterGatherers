//
// Created by axelw on 2019-01-05.
//

#ifndef HUNTERGATHERERS_HEART_H
#define HUNTERGATHERERS_HEART_H


#include "WorldObject.h"

class Heart : public WorldObject {
public:
    Heart(World *world, const sf::Vector2f &position);
    static void loadResources();

    void update(float deltaTime) override;
    void draw(sf::RenderWindow *window, float deltaTime) override;

private:
    sf::Sprite sprite;

    static bool loaded;
    static sf::Texture texture;

};


#endif //HUNTERGATHERERS_HEART_H
