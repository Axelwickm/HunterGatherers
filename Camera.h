//
// Created by Axel on 2018-11-23.
//

#ifndef HUNTERGATHERERS_CAMERA_H
#define HUNTERGATHERERS_CAMERA_H


#include <SFML/Graphics.hpp>
#include <cmath>

#include "Config.h"

class Camera {
public:
    explicit Camera(sf::RenderWindow *window, sf::Vector2f worldSize);
    void move(sf::Vector2f offset);
    void zoomTo(float mouseWheelDelta, sf::Vector2<int> mousePosition);
    void resizeWindow(sf::Event::SizeEvent size);

    sf::View getView();
    void setView(sf::View view);
private:
    sf::RenderWindow *window;
    sf::View view;
    sf::Vector2f worldSize;
};


#endif //HUNTERGATHERERS_CAMERA_H
