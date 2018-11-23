//
// Created by Axel on 2018-11-23.
//

#include "World.h"
#include "BouncingBall.h"

World::World(sf::RenderWindow *window, sf::Vector2f dimensions):
window(window), dimensions(dimensions), quadtree(Quadtree<float>(sf::Vector2<float>(0, 0), dimensions)) {
    quadtree.setLimit(30);

    long long int now = std::chrono::time_point_cast<std::chrono::microseconds>(
            std::chrono::system_clock::now()).time_since_epoch().count();

    srand(now);
    for (int i = 0; i < 100; i++) {
        sf::Vector2<float> position(rand() % ((int) dimensions.x - 50) + 25, rand() % ((int) dimensions.y - 50) + 25);
        std::shared_ptr<BouncingBall> w(new BouncingBall(position, 10.f, dimensions));
        w->setVelocity({(float) rand() / RAND_MAX * 50.f - 25.f, (float) rand() / RAND_MAX * 50.f - 25.f});
        addObject(w);
    }
}

void World::update(float deltaTime) {
    for (auto &object : objects) {
        object->update(deltaTime);
    }
}

void World::draw(float deltaTime) {
    for (auto &object : objects) {
        object->draw(window, deltaTime);
    }

    if (RenderSettings::showQuadtree){
        quadtree.draw(window, RenderSettings::showQuadtreeEntities);
    }
}

bool World::addObject(std::shared_ptr<WorldObject> worldObject) {
    if (quadtree.add(worldObject)){
        worldObject->setQuadtree(&quadtree, worldObject);
        objects.insert(worldObject);
        return true;
    }
    return false;
}

bool World::removeObject(WorldObject* worldObject) {
    if (quadtree.remove(worldObject)){
        objects.erase(std::shared_ptr<WorldObject>(worldObject));
        return true;
    }
    return false;
}
