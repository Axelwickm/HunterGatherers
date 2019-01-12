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
    explicit WorldObject(std::string type, World *world, sf::Vector2f position, bool collider);

    WorldObject(const WorldObject &other);
    const std::string type;
    std::shared_ptr<WorldObject> getSharedPtr();

    virtual void update(float deltaTime);
    virtual void update(float deltaTime, sf::Vector2f oldPosition);
    virtual void draw(sf::RenderWindow *window, float deltaTime);

    void setQuadtree(Quadtree<float> *quadtree, std::weak_ptr<WorldObject> object);
    Quadtree<float> *getQuadtree();

    const sf::Vector2f &getVelocity() const;
    void setVelocity(const sf::Vector2f &velocity);

    float getMass() const;
    void setMass(float accelerationFactor);

    float getFriction() const;

    void setFriction(float friction);

    void applyForce(float deltaTime, sf::Vector2f force);

    const sf::IntRect getWorldBounds() const;
    const sf::FloatRect getWorldBoundsf() const;

    const sf::IntRect &getBounds() const;
    void setBounds(const sf::IntRect &bounds);

    const sf::Vector2f &getPosition() const;
    void setPosition(const sf::Vector2f &position);

    const bool isCollider() const;

    float getAge() const;

    void setAge(float age);

    const sf::Color &getColor() const;
    void setColor(const sf::Color &color);

protected:
    World *world;
    std::weak_ptr<WorldObject> me;
    const bool collider;
    float age;
    sf::Color color;

    Quadtree<float> *quadtree;
    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::IntRect bounds;

    float mass;
    float friction;


};



#endif //FAMILYISEVERYTHING_WORLDOBJECT_H
