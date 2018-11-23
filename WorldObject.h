//
// Created by Axel on 2018-11-21.
//

#ifndef FAMILYISEVERYTHING_WORLDOBJECT_H
#define FAMILYISEVERYTHING_WORLDOBJECT_H

#include <SFML/Graphics.hpp>

#include <memory>


template<class T>
class Quadtree;
class World;

class WorldObject {
public:
    explicit WorldObject(World* world, sf::Vector2f position);
    std::shared_ptr<WorldObject> getSharedPtr();

    virtual void update(float deltaTime);
    virtual void update(float deltaTime, sf::Vector2f oldPosition);
    virtual void draw(sf::RenderWindow *window, float deltaTime) = 0;



    void setQuadtree(Quadtree<float> *quadtree, std::weak_ptr<WorldObject> object);
    Quadtree<float> *getQuadtree();

    const sf::Vector2f &getVelocity() const;
    void setVelocity(const sf::Vector2f &velocity);

    float getAccelerationFactor() const;
    void setAccelerationFactor(float accelerationFactor);



protected:
    World* world;
    std::weak_ptr<WorldObject> me;
    Quadtree<float> *quadtree;

    sf::Vector2f position;
public:
    const sf::Vector2f &getPosition() const;

    void setPosition(const sf::Vector2f &position);

protected:
    sf::Vector2f velocity;
    float accelerationFactor;


private:

};


#endif //FAMILYISEVERYTHING_WORLDOBJECT_H
