//
// Created by Axel on 2018-11-21.
//

#include <memory>
#include <vector>
#include <array>
#include <cmath>
#include <SFML/Graphics.hpp>

#include "WorldObject.h"

#ifndef FAMILYISEVERYTHING_QUADTREE_H
#define FAMILYISEVERYTHING_QUADTREE_H

template<class T>
class Quadtree {
public:
    Quadtree(sf::Vector2<T> topLeft, sf::Vector2<T> dimensions);

    sf::Vector2<T> getPosition();

    sf::Vector2<T> getDimensions();

    std::array<Quadtree<T> *, 4> getQuads();

    bool hasQuads();

    std::vector<std::shared_ptr<WorldObject> > getNodes();

    unsigned long long getSubNodeCount();

    std::vector<std::shared_ptr<WorldObject> > searchNear(sf::Vector2<T> position, float distance);

    T getLimit();

    void setLimit(T l);

    bool contains(sf::Vector2<T> position);

    bool add(std::shared_ptr<WorldObject> worldObject);

    bool remove(WorldObject *worldObject);

    bool move(sf::Vector2f oldPosition, std::shared_ptr<WorldObject> worldObject);


private:
    sf::Vector2<T> topLeft;
    sf::Vector2<T> dimensions;
    T limit;
    std::array<std::unique_ptr<Quadtree<T> >, 4> quads;
    bool quadsCreated;

    std::vector<std::shared_ptr<WorldObject> > nodes;

    bool boxesIntersect(sf::Vector2<T> apos, sf::Vector2<T> adim, sf::Vector2<T> bpos, sf::Vector2<T> bdim);
};


#endif //FAMILYISEVERYTHING_QUADTREE_H
