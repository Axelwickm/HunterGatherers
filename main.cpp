#include <iostream>
#include <memory>
#include <numeric>

#include "Camera.h"
#include "Config.h"
#include "World.h"


int main() {
    const sf::Vector2f dimensions(1920, 1080);

    sf::RenderWindow window(sf::VideoMode(dimensions.x, dimensions.y), "Hunter Gatherers");
    Camera camera(&window, dimensions);
    World world(&window, dimensions);

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

        // Updating world
        world.update(dt.asSeconds());

        // Drawing

        window.clear(sf::Color::Black);

        world.draw(dt.asSeconds());

        window.display();
    }

    std::cout << "Average time: " << std::accumulate(times.begin(), times.end(), 0.0) / times.size() << std::endl;

    return 0;
}