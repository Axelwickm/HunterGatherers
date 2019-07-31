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

template <class T>
inline bool pointInBox(const sf::Vector2<T> p, const sf::Rect<T> a){
    return a.left < p.x && a.top < p.y
           && p.x <= a.left + a.width && p.y <= a.top + a.height;
}

template<class T>
inline bool clipT(const T &n, const T &d, sf::Vector2<T> *c) {
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

inline sf::Color colorFromGenome(const std::vector<double>& genome){
    std::vector<double> reducedGenome(3);
    for (std::size_t i = 0; i < 3; i++){
        reducedGenome.push_back(0);
        for (std::size_t j = 0; j < 3; j++){
            reducedGenome.at(i) += genome.at((3*i+j) % (genome.size()-1)) / 3;
        }
    }
    return sf::Color(reducedGenome[0]*250+5, reducedGenome[1]*250+5, reducedGenome[2]*250+5);
}

template <class T>
class Contiguous2dVector {
public:
    Contiguous2dVector(std::size_t n, std::size_t m, const T fillValue)
            : fillValue(fillValue), n(n), m(m) {
        data = std::vector<T>(n*m, fillValue);
    }

    explicit Contiguous2dVector(const T fillValue)
    : fillValue(fillValue), n(0), m(0) {}

    Contiguous2dVector<T>& operator=(const Contiguous2dVector& other){
        n = other.n; m = other.m;
        data = other.data;
        return *this;
    }

    void push_back_row(std::vector<T> item){
        if (m < item.size()){
            // Insert fill value after every row
            const std::size_t nm = item.size();
            data.reserve(n*nm);
            for (std::size_t i = 0; i < n; i++){
                // FIXME: +1 v?
                data.insert(std::begin(data)+m+i*nm, nm-m, fillValue);
            }
            m = nm;
        }

        // Add new data
        data.insert(std::end(data), std::begin(item), std::end(item));
        n++;
    }

    void clear(){
        data.clear();
        n = 0; m = 0;
    }

    std::size_t getN() const {
        return n;
    }

    std::size_t getM() const {
        return m;
    }

    T& at(std::size_t x, std::size_t y){
        if (n <= x)
            throw std::out_of_range("x ("+std::to_string(x)+") too big for n ("+std::to_string(n)+")");
        if (m <= y)
            throw std::out_of_range("y ("+std::to_string(x)+") too big for m ("+std::to_string(n)+")");

        return data.at(x*m+y);
    }

    std::pair<typename std::vector<T>::iterator, typename std::vector<T>::iterator> at(std::size_t x){
        if (n <= x)
            throw std::out_of_range("x ("+std::to_string(x)+") too big for n ("+std::to_string(n)+")");

        return std::make_pair(std::begin(data)+x*m, std::begin(data)+x*m+m);
    }

    const T getFillValue() const {
        return fillValue;
    }

private:
    std::size_t n, m;
    const T fillValue;
    std::vector<T> data;
};

template class Contiguous2dVector<sf::Color>;

#endif //FAMILYISEVERYTHING_UTILS_CPP