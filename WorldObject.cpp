//
// Created by Axel on 2018-11-21.
//

#include "Agent.h"
#include "WorldObject.h"
#include "Quadtree.h"
#include "World.h"

WorldObject::WorldObject(std::string type, World *world, sf::Vector2f position, bool collider)
: type(std::move(type)), collider(collider) {
    this->position = position;
    this->quadtree = nullptr;
    this->world = world;
    velocity = sf::Vector2f(0, 0);
    mass = 1.f;
    friction = 0.f;
    bounds = sf::IntRect(0, 0, 0, 0);
}

WorldObject::WorldObject(const WorldObject &other)
: type(other.type), collider(other.collider) {
    this->position = other.position;
    this->quadtree = other.quadtree;
    this->world = other.world;
    velocity = other.velocity;
    mass = other.mass;
    friction = other.friction;
    bounds = other.bounds;
}

std::shared_ptr<WorldObject> WorldObject::getSharedPtr() {
    return std::shared_ptr<WorldObject>(me);
}

void WorldObject::setQuadtree(Quadtree<float> *quadtree, std::weak_ptr<WorldObject> object) {
    this->quadtree = quadtree;
    this->me = object;
}

Quadtree<float> *WorldObject::getQuadtree() {
    return quadtree;
}

void WorldObject::update(float deltaTime) {
    sf::Vector2f old = position;
    update(deltaTime, old);
}

void WorldObject::update(float deltaTime, sf::Vector2f oldPosition) {
    velocity *= powf(friction, deltaTime);
    position += velocity * deltaTime;
    position = sf::Vector2f((float) fmin(position.x, world->getDimensions().x), (float) fmin(position.y, world->getDimensions().y));
    position = sf::Vector2f((float) fmax(position.x, 1), (float) fmax(position.y, 1));
    if (quadtree != nullptr) {
        quadtree->move(oldPosition, this);
    }

}

void WorldObject::applyForce(float deltaTime, const sf::Vector2f force) {
    velocity += force/mass*deltaTime;
}

void WorldObject::draw(sf::RenderWindow *window, float deltaTime) {
    if (world->getConfig().render.showWorldObjectBounds){
        sf::VertexArray rect(sf::LineStrip, 5);
        sf::IntRect b = getBounds();
        rect[0].position = sf::Vector2f(getPosition() + sf::Vector2f(b.left, b.top));
        rect[1].position = sf::Vector2f(getPosition() + sf::Vector2f(b.width, b.top));
        rect[2].position = sf::Vector2f(getPosition() + sf::Vector2f(b.width, b.height));
        rect[3].position = sf::Vector2f(getPosition() + sf::Vector2f(b.left, b.height));
        rect[4].position = sf::Vector2f(getPosition() + sf::Vector2f(b.left, b.top));

        rect[0].color = sf::Color(200, 200, 200);
        rect[1].color = sf::Color(200, 200, 200);
        rect[2].color = sf::Color(200, 200, 200);
        rect[3].color = sf::Color(200, 200, 200);
        rect[4].color = sf::Color(200, 200, 200);
        window->draw(rect);
    }
}

const sf::Vector2f &WorldObject::getPosition() const {
    return position;
}

void WorldObject::setPosition(const sf::Vector2f &position) {
    WorldObject::position = position;
}


const sf::Vector2f &WorldObject::getVelocity() const {
    return velocity;
}

void WorldObject::setVelocity(const sf::Vector2f &velocity) {
    WorldObject::velocity = velocity;
}

float WorldObject::getMass() const {
    return mass;
}

void WorldObject::setMass(float accelerationFactor) {
    WorldObject::mass = accelerationFactor;
}

const sf::IntRect &WorldObject::getBounds() const {
    return bounds;
}

const sf::IntRect WorldObject::getWorldBounds() const {
    return sf::IntRect(getPosition().x + getBounds().left,
                       getPosition().y + getBounds().top,
                       getBounds().width - getBounds().left,
                       getBounds().height - getBounds().top);
}

const sf::FloatRect WorldObject::getWorldBoundsf() const {
    return sf::FloatRect(getPosition().x + getBounds().left,
                       getPosition().y + getBounds().top,
                       getBounds().width - getBounds().left,
                       getBounds().height - getBounds().top);
}



void WorldObject::setBounds(const sf::IntRect &bounds) {
    WorldObject::bounds = bounds;
}

const bool WorldObject::isCollider() const {
    return collider;
}

float WorldObject::getFriction() const {
    return friction;
}

void WorldObject::setFriction(float friction) {
    WorldObject::friction = friction;
}


const sf::Color &WorldObject::getColor() const {
    return color;
}

void WorldObject::setColor(const sf::Color &color) {
    WorldObject::color = color;
}
