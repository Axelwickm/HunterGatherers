//
// Created by Axel on 2018-11-23.
//

#ifndef FAMILYISEVERYTHING_UTILS_CPP
#define FAMILYISEVERYTHING_UTILS_CPP

#include <SFML/Graphics.hpp>

#define EPSILON 1e-6f

template<class T>
inline bool boxesIntersect(const sf::Rect<T> a, const sf::Rect<T> b){
    return !(b.left > a.left + a.width
             || b.left + b.width < a.left
             || b.top > a.top + a.height
             || b.top + b.width < a.top);
}

template<class T>
inline bool clipT(const T &n, const T &d, sf::Vector2<T>* c){
    sf::Vector2<T> c2(*c);
    if (abs(d) < EPSILON){
        return n < 0;
    }
    T t = n / d;

    if (0 < d) {
        if (t > c2.y) return false;
        if (t > c2.x) (*c).x = t;
    } else {
        if (t < c2.x) return false;
        if (t < c2.y) (*c).y = t;
    }
    return true;
}

template<class T>
inline bool lineIntersectWithBox(const sf::Vector2<T> &lineStart, const sf::Vector2<T> &lineEnd, const sf::Vector2<T> &boxpos, const sf::Vector2<T> &boxdim){
    T box[4] = {boxpos.x, boxpos.y, boxpos.x + boxdim.x, boxpos.y + boxdim.y};
    T dx = lineEnd.x - lineStart.x;
    T dy = lineEnd.y - lineStart.y;

    if (dx < EPSILON && dy < EPSILON &&
        lineStart.x >= box[0] && lineStart.x <= box[2] &&
        lineStart.y >= box[1] && lineStart.y <= box[3]){
        return true;
    }

    sf::Vector2<T> c = {0, 1};
    return clipT(box[0] - lineStart.x, dx, &c) &&
           clipT(lineStart.x - box[2], -dx, &c) &&
           clipT(box[1] - lineStart.y,  dy, &c) &&
           clipT(lineStart.y - box[3], -dy, &c);


}

#endif //FAMILYISEVERYTHING_UTILS_CPP