//
// Created by Axel on 2018-11-23.
//

#ifndef FAMILYISEVERYTHING_UTILS_CPP
#define FAMILYISEVERYTHING_UTILS_CPP

#include <SFML/Graphics.hpp>

template<class T>
bool boxesIntersect(sf::Vector2<T> apos, sf::Vector2<T> adim, sf::Vector2<T> bpos, sf::Vector2<T> bdim) {
    return (fabs(apos.x - bpos.x) * 1.f < (adim.x + bdim.x)) &&
           (fabs(apos.y - bpos.y) * 1.f < (adim.y + bdim.y));
}

#endif //FAMILYISEVERYTHING_UTILS_CPP