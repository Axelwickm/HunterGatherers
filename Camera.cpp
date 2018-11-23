//
// Created by Axel on 2018-11-23.
//

#include "Camera.h"

Camera::Camera(sf::RenderWindow *window) {
    this->window = window;
    this->view = window->getDefaultView();
    view.zoom(1.f);
    window->setView(view);
}

void Camera::move(sf::Vector2f offset) {
    offset *= view.getSize().x/window->getSize().x;
    view.move(offset.x, offset.y);

    view.setCenter((float) fmin(view.getCenter().x, window->getSize().x), (float) fmin(view.getCenter().y, window->getSize().y)); // TODO: change this to world dimensions
    view.setCenter((float) fmax(view.getCenter().x, 0), (float) fmax(view.getCenter().y, 0));

    window->setView(view);
}

void Camera::zoomTo(float mouseWheelDelta,  sf::Vector2<int> mousePosition) {
    sf::Vector2f c1 = (sf::Vector2f) window->mapPixelToCoords(sf::Mouse::getPosition(*window));
    view.zoom(1.f-mouseWheelDelta * Controls::scrollFactor);
    view.setSize((float) fmin(view.getSize().x, window->getSize().x), (float) fmin(view.getSize().y, window->getSize().y));
    auto c2 = (sf::Vector2f) window->mapPixelToCoords(mousePosition, view);
    view.move(c1-c2);

    view.setCenter((float) fmin(view.getCenter().x, window->getSize().x), (float) fmin(view.getCenter().y, window->getSize().y)); // TODO: change this to world dimensions
    view.setCenter((float) fmax(view.getCenter().x, 0), (float) fmax(view.getCenter().y, 0));

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



