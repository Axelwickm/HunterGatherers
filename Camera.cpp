//
// Created by Axel on 2018-11-23.
//

#include "Camera.h"

Camera::Camera(Config &config, sf::RenderWindow *window, sf::Vector2f worldSize) : config(config) {
    this->window = window;
    this->view = window->getDefaultView();
    this->worldSize = worldSize;
    view.zoom(1.f);
    window->setView(view);
}

void Camera::move(sf::Vector2f offset) {
    offset *= view.getSize().x/window->getSize().x;
    view.move(offset.x, offset.y);

    view.setCenter((float) fmin(view.getCenter().x, worldSize.x), (float) fmin(view.getCenter().y, worldSize.y));
    view.setCenter((float) fmax(view.getCenter().x, 0), (float) fmax(view.getCenter().y, 0));

    window->setView(view);
}

void Camera::zoomTo(float mouseWheelDelta,  sf::Vector2<int> mousePosition) {
    sf::Vector2f c1 = (sf::Vector2f) window->mapPixelToCoords(sf::Mouse::getPosition(*window));
    view.zoom(1.f-mouseWheelDelta * config.controls.scrollFactor);
    view.setSize((float) fmin(view.getSize().x, worldSize.x), (float) fmin(view.getSize().y, worldSize.y));
    auto c2 = (sf::Vector2f) window->mapPixelToCoords(mousePosition, view);
    view.move(c1-c2);

    //view.setCenter((float) fmin(view.getCenter().x, window->getSize().x), (float) fmin(view.getCenter().y, window->getSize().y));
    //view.setCenter((float) fmax(view.getCenter().x, 0), (float) fmax(view.getCenter().y, 0));

    window->setView(view);
}

void Camera::resizeWindow(sf::Event::SizeEvent size) {
    float aspectRatio = (float) size.width / size.height;
    sf::Vector2f v = view.getSize();
    view.setSize(v.x, v.x/aspectRatio);
    window->setView(view);
}

sf::View Camera::getView() {
    return view;
}

void Camera::setView(sf::View view) {
    this->view = view;
    window->setView(view);
}



