//
// Created by Axel on 2018-11-21.
//

#include <memory>
#include <vector>
#include <array>
#include <cmath>
#include <SFML/Graphics.hpp>

#include "WorldObject.h"
#include "utils.cpp"

#ifndef FAMILYISEVERYTHING_QUADTREE_H
#define FAMILYISEVERYTHING_QUADTREE_H

template<class T>
class Quadtree {
public:
    Quadtree(sf::Vector2<T> topLeft, sf::Vector2<T> dimensions);


    std::array<Quadtree<T> *, 4> getQuads();
    bool hasQuads();

    sf::Vector2<T> getPosition();
    sf::Vector2<T> getDimensions();

    T getLimit();
    void setLimit(T l);

    std::vector<std::shared_ptr<WorldObject> > searchNear(sf::Vector2<T> position, float distance); // TODO: do same as below
    void searchNearLine(std::vector<std::shared_ptr<WorldObject> > &wobjs, const sf::Vector2<T> &lineStart, const sf::Vector2<T> &lineEnd);
    bool contains(sf::Vector2<T> position);

    unsigned long long getSubNodeCount();
    std::vector<std::shared_ptr<WorldObject> > getNodes();

    bool add(std::shared_ptr<WorldObject> worldObject);
    bool remove(WorldObject *worldObject);
    bool move(sf::Vector2f oldPosition, WorldObject* worldObject);

    void draw(sf::RenderWindow *window, bool entities);


private:
    sf::Vector2<T> topLeft;
    sf::Vector2<T> dimensions;
    T limit;
    std::array<std::unique_ptr<Quadtree<T> >, 4> quads;
    bool quadsCreated;

    std::vector<std::shared_ptr<WorldObject> > nodes;
};


#endif //FAMILYISEVERYTHING_QUADTREE_H
