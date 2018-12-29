#include <iostream>
#include <memory>
#include <string>

#include "Camera.h"
#include "Config.h"
#include "World.h"
#include "OpenCL_Wrapper.h"

int main(int argc, char *argv[]) {
    std::string deviceName;

    for (int i = 0; i < argc; i++){
        std::string s(argv[i]);
        if (s == "-CL_DEVICE"){
            deviceName = std::string(argv[i+1]);
            while (true){
                auto c = deviceName.find("_");
                if (c == std::string::npos){
                    break;
                }
                deviceName.replace(c, 1, " ");
            }

            printf("Set to use OpenCL device: %s\n", deviceName.c_str());

            i++;
        }
    }

    OpenCL_Wrapper cl(deviceName);

    const sf::Vector2f worldDimensions(10000, 10000);


    sf::RenderWindow window(sf::VideoMode(1920, 1080), "Hunter Gatherers");
    Camera camera(&window, sf::Vector2f(1920*10, 1080*10));
    World world(&window, worldDimensions, &cl);

    sf::Clock deltaClock;
    bool paused = false;

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
                if (code == Controls::pause){
                    paused = !paused;
                }
                else if (code == Controls::close){
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

            // Mouse drag
            else if (event.type == sf::Event::MouseMoved){
                dragging = true;
                /*if (sf::Touch::isDown(0)){
                    std::cout<<"Touch\n";
                    mousePosition = sf::Touch::getPosition(0);
                }*/
                //std::cout<<sf::Touch::isDown(1)<<std::endl;
                if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)){
                    camera.move((sf::Vector2f) (mousePosition - sf::Vector2<int>(event.mouseMove.x, event.mouseMove.y)));
                }
                mousePosition = sf::Vector2<int>(event.mouseMove.x, event.mouseMove.y);

            }

            // Mouse was scrolled
            else if (event.type == sf::Event::MouseWheelScrolled){
                sf::Vector2<int> c =  sf::Mouse::getPosition(window);
                camera.zoomTo(event.mouseWheelScroll.delta, c);
            }

            // A finger was placed on the screen
            else if (event.type == sf::Event::TouchBegan){
                std::cout<<"Touch!\n";
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
        if (!paused){
            world.update(dt.asSeconds());
        }

        // Drawing

        window.clear(sf::Color::Black);

        world.draw(dt.asSeconds());

        window.display();
    }

    return 0;
}