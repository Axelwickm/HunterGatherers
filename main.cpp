#include <iostream>
#include <random>
#include <chrono>
#include <memory>

#include "Quadtree.h"
#include "BouncingBall.h"
#include "Camera.h"
#include "Config.h"

template<class T>
void drawQuadtree(sf::RenderWindow *window, Quadtree<T> *q) {
    sf::CircleShape c;
    c.setRadius(5);
    c.setFillColor(sf::Color::Red);
    c.setOrigin(c.getRadius(), c.getRadius());

    if (RenderSettings::showQuadtreeEntities) {
        for (const std::shared_ptr<WorldObject> &wo : q->getNodes()) {
            c.setPosition(sf::Vector2f(wo->getPosition()));
            window->draw(c);
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

    sf::RenderWindow window(sf::VideoMode(dimensions.x, dimensions.y), "Hunter Gatherers");
    Camera camera(&window);

    long long int now = std::chrono::time_point_cast<std::chrono::microseconds>(
            std::chrono::system_clock::now()).time_since_epoch().count();

    srand(now);
    for (int i = 0; i < 100; i++) {
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

    bool dragging = false;
    sf::Vector2<int> mousePosition = sf::Mouse::getPosition();

    // Game loop
    while (window.isOpen()) {
        sf::Time dt = deltaClock.restart();

        // Get events
        sf::Event event{};
        while (window.pollEvent(event)) {

            // A key was pressed
            if (event.type == sf::Event::KeyPressed){
                sf::Keyboard::Key code = event.key.code;
                if (code == Controls::close){
                    window.close();
                }
            }

            // Mouse pressed
            else if (event.type == sf::Event::MouseButtonPressed){
                if (event.mouseButton.button == sf::Mouse::Button::Left){
                    dragging = false;
                }
            }

            // Mouse released
            else if (event.type == sf::Event::MouseButtonReleased){
                if (event.mouseButton.button == sf::Mouse::Button::Left && !dragging){
                    // Click!
                    std::cout<<"Click!\n";
                }
            }

            else if (event.type == sf::Event::MouseMoved){
                dragging = true;
                if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)){
                    camera.move((sf::Vector2f) (mousePosition - sf::Vector2<int>(event.mouseMove.x, event.mouseMove.y)));
                }
                mousePosition = sf::Vector2<int>(event.mouseMove.x, event.mouseMove.y);

            }

                // Mouse was scrolled
            else if (event.type == sf::Event::MouseWheelScrolled){
                camera.zoomTo(event.mouseWheelScroll.delta, sf::Mouse::getPosition(window));
            }

            // Window resized
            else if (event.type == sf::Event::Resized){
                camera.resizeWindow(event.size);
            }

            // Close window
            else if (event.type == sf::Event::Closed){
                window.close();
            }
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
    }

    std::cout << "Average: " << std::accumulate(times.begin(), times.end(), 0.0) / times.size() << std::endl;

    return 0;
}