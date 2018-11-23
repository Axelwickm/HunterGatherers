//
// Created by Axel on 2018-11-21.
//

#ifndef FAMILYISEVERYTHING_WORLDOBJECT_H
#define FAMILYISEVERYTHING_WORLDOBJECT_H

#include <SFML/Graphics.hpp>

#include <memory>

template<class T>
class Quadtree;

class WorldObject {
public:
    explicit WorldObject(sf::Vector2f position);
    std::shared_ptr<WorldObject> getSharedPtr();

    virtual void update(float deltaTime) = 0;
    virtual void draw(sf::RenderWindow *window, float deltaTime) = 0;

    sf::Vector2f getPosition();
    void setPosition(sf::Vector2f position);

    void setQuadtree(Quadtree<float> *quadtree, std::weak_ptr<WorldObject> object);
    Quadtree<float> *getQuadtree();

protected:
    std::weak_ptr<WorldObject> me;
    sf::Vector2f position;

    Quadtree<float> *quadtree;
private:

};


#endif //FAMILYISEVERYTHING_WORLDOBJECT_H
