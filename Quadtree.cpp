//
// Created by Axel on 2018-11-21.
//

#include "Quadtree.h"

template<class T>
Quadtree<T>::Quadtree(sf::Vector2<T> topLeft, sf::Vector2<T> dimensions) {
    this->topLeft = topLeft;
    this->dimensions = dimensions;
    limit = 10;
    quadsCreated = false;
}

template<class T>
sf::Vector2<T> Quadtree<T>::getPosition() {
    return topLeft;
}

template<class T>
sf::Vector2<T> Quadtree<T>::getDimensions() {
    return dimensions;
}

template<class T>
std::array<Quadtree<T> *, 4> Quadtree<T>::getQuads() {
    std::array<Quadtree<T> *, 4> regularPointers{};
    for (int i = 0; i < 4; i++) {
        regularPointers[i] = quads[i].get();
    }
    return regularPointers;
}

template<class T>
std::vector<std::shared_ptr<WorldObject> > Quadtree<T>::getNodes() {
    return nodes;
}

template<class T>
unsigned long long Quadtree<T>::getSubNodeCount() {
    unsigned long long c = 0;
    if (quadsCreated) {
        for (int i = 0; i < 4; i++) {
            c += quads[i]->getSubNodeCount();
        }
    }
    return c + nodes.size();
}

template<class T>
T Quadtree<T>::getLimit() {
    return limit;
}

template<class T>
void Quadtree<T>::setLimit(T l) {
    limit = l;
}

template<class T>
bool Quadtree<T>::hasQuads() {
    return quadsCreated;
}

template<class T>
bool Quadtree<T>::add(std::shared_ptr<WorldObject> worldObject) {
    // Return false beacuse point is outside quadtree
    if (!contains(worldObject->getPosition())) {
        return false;
    }

    // If there are no child-quads
    if (not quadsCreated) {
        // Add node here if the tree can't be divided further
        if (dimensions.x / 2 < limit || dimensions.y / 2 < limit) {
            nodes.push_back(worldObject);
            return true;
        }
            // Divide the tree
        else {
            // Top left
            quads[0] = std::unique_ptr<Quadtree<T> >(new Quadtree(topLeft, dimensions / (T) 2));
            quads[0]->setLimit(limit);
            // Top right
            quads[1] = std::unique_ptr<Quadtree<T> >(
                    new Quadtree(topLeft + sf::Vector2<T>(dimensions.x / 2, 0), dimensions / (T) 2));
            quads[1]->setLimit(limit);
            // Bottom left
            quads[2] = std::unique_ptr<Quadtree<T> >(
                    new Quadtree(topLeft + sf::Vector2<T>(0, dimensions.y / 2), dimensions / (T) 2));
            quads[2]->setLimit(limit);
            // Bottom right
            quads[3] = std::unique_ptr<Quadtree<T> >(
                    new Quadtree(topLeft + sf::Vector2<T>(dimensions.x / 2, dimensions.y / 2), dimensions / (T) 2));
            quads[3]->setLimit(limit);

            quadsCreated = true;

        }
    }

    // Add the node
    for (int i = 0; i < 4; i++) {
        if (quads[i]->add(worldObject)) {
            return true;
        }
    }

    return false;
}

template<class T>
bool Quadtree<T>::remove(WorldObject *worldObject) {
    // Does not contain
    if (!contains(worldObject->getPosition())) {
        return false;
    }

    // Does contain
    if (not quadsCreated) {
        for (int i = 0; i < nodes.size(); i++) {
            if (nodes.at(i).get() == worldObject) {
                nodes.erase(nodes.begin() + i);
                return true;
            }
        }
        return false;
    }

    // Remove from quads
    bool removed = false;
    for (int i = 0; i < 4; i++) {
        if (quads[i]->remove(worldObject)) {
            removed = true;
            break;
        }
    }

    if (removed) {
        // Check if it is even worth having quads, or if they should be deconstruced
        if (quadsCreated and getSubNodeCount() == 0) {
            quadsCreated = false;
            for (int i = 0; i < 4; i++) {
                quads[i].reset();
            }
        }
        return true;
    }

    return false;
}

template<class T>
bool Quadtree<T>::move(sf::Vector2f oldPosition, WorldObject* worldObject) {
    // Does not contain
    if (!contains(oldPosition)) {
        return false;
    }

    if (not quadsCreated) {
        for (unsigned long long int i = 0; i < nodes.size(); i++) {
            if (nodes.at(i).get() == worldObject) {
                if (!contains(worldObject->getPosition())) {
                    nodes.erase(nodes.begin() + i);
                    return true;
                }
                return false;
            }
        }
        return false;
    }

    // Remove from quads
    for (int i = 0; i < 4; i++) {
        if (quads[i]->move(oldPosition, worldObject)) {
            if (add(worldObject->getSharedPtr())) {
                return false;
            } else {
                // Check if it is even worth having quads, or if they should be deconstruced
                if (quadsCreated and getSubNodeCount() == 0) {
                    quadsCreated = false;
                    for (int i = 0; i < 4; i++) {
                        quads[i].reset();
                    }
                }

                return true;
            }
        }
    }

    return false;
}

template<class T>
bool Quadtree<T>::contains(sf::Vector2<T> position) {
    return topLeft.x < position.x && topLeft.y < position.y
           && position.x <= topLeft.x + dimensions.x && position.y <= topLeft.y + dimensions.y;
}

template<class T>
std::vector<std::shared_ptr<WorldObject> > Quadtree<T>::searchNear(sf::Vector2<T> position, float distance) {
    if (boxesIntersect(topLeft, dimensions, sf::Vector2<T>(position.x - distance, position.y - distance),
                       sf::Vector2<T>(2 * distance, 2 * distance))) {
        std::vector<std::shared_ptr<WorldObject> > n1 = nodes;

        if (quadsCreated) {
            for (int i = 0; i < 4; i++) {
                std::vector<std::shared_ptr<WorldObject> > n2 = quads[i]->searchNear(position, distance);
                n1.insert(n1.end(), n2.begin(), n2.end());
            }
        }

        return n1;
    }
    return std::vector<std::shared_ptr<WorldObject> >();
}

template<class T>
void Quadtree<T>::searchNearLine(std::vector<std::shared_ptr<WorldObject> > &wobjs, const sf::Vector2<T> &lineStart, const sf::Vector2<T> &lineEnd) {
    if (lineIntersectWithBox(lineStart, lineEnd, topLeft, dimensions)) {
        if (!nodes.empty()){
            wobjs.insert(wobjs.end(), nodes.begin(), nodes.end());
        }

        if (quadsCreated) {
            for (int i = 0; i < 4; i++) {
                quads[i]->searchNearLine(wobjs, lineStart, lineEnd);
            }
        }


    }
}

template<class T>
void Quadtree<T>::draw(sf::RenderWindow *window, bool entities) {
    sf::CircleShape c;
    c.setRadius(5);
    c.setFillColor(sf::Color::Red);
    c.setOrigin(c.getRadius(), c.getRadius());

    if (entities) {
        for (const std::shared_ptr<WorldObject> &wo : getNodes()) {
            c.setPosition(sf::Vector2f(wo->getPosition()));
            window->draw(c);
        }
    }
    sf::VertexArray rect(sf::LineStrip, 5);
    rect[0].position = sf::Vector2f(getPosition());
    rect[1].position = sf::Vector2f(getPosition()) + sf::Vector2f(0, getDimensions().y);
    rect[2].position = sf::Vector2f(getPosition()) + sf::Vector2f(getDimensions().x, getDimensions().y);
    rect[3].position = sf::Vector2f(getPosition()) + sf::Vector2f(getDimensions().x, 0);
    rect[4].position = sf::Vector2f(getPosition());
    rect[0].color = sf::Color(100, 100, 100);
    rect[1].color = sf::Color(100, 100, 100);
    rect[2].color = sf::Color(100, 100, 100);
    rect[3].color = sf::Color(100, 100, 100);
    rect[4].color = sf::Color(100, 100, 100);
    window->draw(rect);

    if (quadsCreated) {
        for (int i = 0; i < 4; i++) {
            quads[i]->draw(window, entities);
        }
    }

}


//template class Quadtree<int>; <-- Triggers division errors
template class Quadtree<float>;
//template class Quadtree<unsigned long>; <-- Triggers division errors