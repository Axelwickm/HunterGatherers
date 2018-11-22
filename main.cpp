#include <iostream>
#include <random>
#include <chrono>
#include <memory>

#include "Quadtree.h"
#include "BouncingBall.h"

std::shared_ptr<WorldObject> toDel = nullptr;

struct RenderSettings {
    bool showQuadtree = true;
    bool showQuadtreeEntities = false;

} renderSettings;

template<class T>
void drawQuadtree(sf::RenderWindow *window, Quadtree<T> *q) {
    sf::CircleShape c;
    c.setRadius(5);
    c.setFillColor(sf::Color::Red);
    c.setOrigin(c.getRadius(), c.getRadius());

    if (renderSettings.showQuadtreeEntities) {
        for (std::shared_ptr<WorldObject> wo : q->getNodes()) {
            c.setPosition(sf::Vector2f(wo->getPosition()));
            window->draw(c);
            if (rand() % 10000 == 0) {
                toDel = wo;
            }
        }
    }

    sf::RectangleShape r;
    r.setPosition(sf::Vector2f(q->getPosition()));
    r.setSize(sf::Vector2f(q->getDimensions()));
    r.setFillColor(sf::Color(0, 0, 0, 0));
    r.setOutlineColor(sf::Color(100, 100, 100));
    r.setOutlineThickness(1);
    window->draw(r);


    if (q->hasQuads()) {
        std::array<Quadtree<T> *, 4> quads = q->getQuads();
        for (int i = 0; i < 4; i++) {
            drawQuadtree(window, quads[i]);
        }
    }

}

int main() {
    const sf::Vector2<float> dimensions(1920, 1080);


    Quadtree<float> q = Quadtree<float>(sf::Vector2<float>(0, 0), dimensions);
    q.setLimit(30);
    std::vector<std::shared_ptr<BouncingBall> > bouncingBalls;

    sf::RenderWindow window(sf::VideoMode(dimensions.x, dimensions.y), "Family is everything");

    long long int now = std::chrono::time_point_cast<std::chrono::microseconds>(
            std::chrono::system_clock::now()).time_since_epoch().count();
    srand(now);
    for (int i = 0; i < 1000; i++) {
        sf::Vector2<float> position(rand() % ((int) dimensions.x - 50) + 25, rand() % ((int) dimensions.y - 50) + 25);
        std::shared_ptr<BouncingBall> w(new BouncingBall(position, 10.f, dimensions));
        w->setVelocity({(float) rand() / RAND_MAX * 50.f - 25.f, (float) rand() / RAND_MAX * 50.f - 25.f});
        w->setQuadtree(&q, (std::weak_ptr<WorldObject>) (w));
        q.add(w);
        bouncingBalls.push_back(w);
    }

    sf::Clock deltaClock;
    sf::Clock computationTime;
    std::vector<sf::Int64> times;

    // Game loop
    while (window.isOpen()) {
        sf::Time dt = deltaClock.restart();

        // Get events
        sf::Event event;
        while (window.pollEvent(event)) {
            // Close window
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear(sf::Color::Black);

        for (auto &ball : bouncingBalls) {
            computationTime.restart();
            ball->update(dt.asSeconds());
            times.push_back(computationTime.getElapsedTime().asMicroseconds());
            ball->draw(&window, dt.asSeconds());
        }

        drawQuadtree(&window, &q);

        window.display();

        if (toDel) {
            q.remove(toDel.get());
            toDel.reset();
        }
    }

    std::cout << "Average: " << std::accumulate(times.begin(), times.end(), 0.0) / times.size() << std::endl;

    return 0;
}