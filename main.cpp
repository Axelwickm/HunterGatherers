#include <iostream>
#include <memory>

#include "Camera.h"
#include "World.h"
#include "GUI.h"

int main(int argc, char *argv[]) {
    printf("Using seed: %lu\n", GeneralSettings::seed);

    // Figure out which OpenCL accelerator device to use
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

    // Create main objects
    sf::RenderWindow window(sf::VideoMode(GeneralSettings::windowSize.x, GeneralSettings::windowSize.y),
            "Hunter Gatherers");
    GUI gui(&window);
    Camera camera(&window, sf::Vector2f(GeneralSettings::windowSize.x*10, GeneralSettings::windowSize.y*10));
    OpenCL_Wrapper cl(deviceName);
    World world(&window, &cl, GeneralSettings::options);

    // Game loop variables

    sf::Clock deltaClock;
    float timeFactor = 1;
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
                if (code == Controls::up){
                    camera.move(sf::Vector2f(0, Controls::upAmount));
                }
                if (code == Controls::down){
                    camera.move(sf::Vector2f(0, Controls::downAmount));
                }
                if (code == Controls::left){
                    camera.move(sf::Vector2f(Controls::leftAmount, 0));
                }
                if (code == Controls::right){
                    camera.move(sf::Vector2f(Controls::rightAmount, 0));
                }
                if (code == Controls::slowDown){
                    timeFactor = fmaxf(Controls::timeFactorDelta, timeFactor-Controls::timeFactorDelta);
                }
                else if (code == Controls::speedUp){
                    timeFactor = fminf(Controls::timeFactorMax, timeFactor+Controls::timeFactorDelta);
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
                    auto mapPos = (sf::Vector2f) window.mapPixelToCoords(mousePosition);
                    auto hits = world.getQuadtree().searchNear(mapPos, 0.1);
                    bool selected = false;
                    for (auto& hit : hits){
                        if (pointInBox(mapPos, hit->getWorldBoundsf())) {
                            if (hit != gui.getSelectedAgent() && typeid(*hit) == typeid(Agent)) {
                                gui.selectAgent(std::dynamic_pointer_cast<Agent>(hit));
                                selected = true;
                                break;
                            }
                        }
                    }
                    if (!selected){
                        gui.selectAgent(nullptr);
                    }
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
            world.update(dt.asSeconds()*timeFactor);
        }

        // Rendering

        window.clear(sf::Color::Black);
        world.draw(paused ? 0 : dt.asSeconds()*timeFactor);
        gui.draw(paused ? 0 : dt.asSeconds()*timeFactor);
        window.display();
    }

    return 0;
}