#include <iostream>
#include <memory>

#include "Camera.h"
#include "World.h"
#include "GUI.h"
#include "MarkovNames.h"

int main(int argc, char *argv[]) {


    // Process potential arguments config filename, and OpenCL device
    std::string deviceName;
    std::string configFilename = ".\\Config.json";

    for (int i = 0; i < argc; i++){
        std::string s(argv[i]);
        if (s == "-CL_DEVICE"){
            deviceName = std::string(argv[i+1]);
            while (true){
                auto c = deviceName.find('_');
                if (c == std::string::npos){
                    break;
                }
                deviceName.replace(c, 1, " ");
            }

            printf("Set to use OpenCL device: %s\n", deviceName.c_str());

            i++;
        }
        else if (s == "-CONFIG"){
            configFilename = std::string(argv[i+1]);
        }
    }
    Config config;
    config.loadConfigFromFile(configFilename);
    printf("Using seed: %lu\n", config.seed);

    MarkovNames::loadResources();
    Gene::randomEngine = std::mt19937(config.seed++);

    // Create main objects
    sf::RenderWindow window(sf::VideoMode(config.render.windowSize.x, config.render.windowSize.y),
            "Hunter Gatherers");
    sf::Image icon;
    icon.loadFromFile("../resources/icon.png");
    window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
    Camera camera(config, &window,
                  sf::Vector2f(config.render.windowSize.x * 10, config.render.windowSize.y * 10));
    OpenCL_Wrapper cl(deviceName);
    World world(config, &window, &cl);
    GUI gui(config, &window, &world, &camera);

    // Game loop variables

    sf::Clock deltaClock;
    float timeFactor = config.controls.timeFactorInitial;
    bool paused = false;

    bool dragging = false;
    bool notMoving = false;
    sf::Vector2<int> mousePosition = sf::Mouse::getPosition();
    float thresholdSpeed = 50;

    const Controls& controls = config.controls;

    // Game loop
    while (window.isOpen()) {
        sf::Time dt = deltaClock.restart();

        // Get events
        sf::Event event{};
        while (window.pollEvent(event)) {
            notMoving = true;

            // A key was pressed
            if (event.type == sf::Event::KeyPressed){
                sf::Keyboard::Key code = event.key.code;
                if (code == controls.pause){
                    paused = !paused;
                }
                else if (code == controls.close){
                    window.close();
                }
                else if (code == controls.showInterface){
                    config.render.showInterface = !config.render.showInterface;
                }
                else if (code == controls.clearStats){
                    world.clearStatistics();
                }
                if (code == controls.up){
                    camera.move(sf::Vector2f(0, controls.upAmount));
                }
                if (code == controls.down){
                    camera.move(sf::Vector2f(0, controls.downAmount));
                }
                if (code == controls.left){
                    camera.move(sf::Vector2f(controls.leftAmount, 0));
                }
                if (code == controls.right){
                    camera.move(sf::Vector2f(controls.rightAmount, 0));
                }
                if (code == controls.slowDown){
                    timeFactor = fmaxf(controls.timeFactorDelta, timeFactor - controls.timeFactorDelta);
                }
                else if (code == controls.speedUp){
                    timeFactor = fminf(controls.timeFactorMax, timeFactor + controls.timeFactorDelta);
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
                    for (auto &hit : hits) {
                        if (pointInBox(mapPos, hit->getWorldBoundsf())) {
                            if (hit != gui.getSelectedAgent() && typeid(*hit) == typeid(Agent)) {
                                gui.selectAgent(std::dynamic_pointer_cast<Agent>(hit));
                                selected = true;
                                break;
                            }
                        }
                    }
                    if (!selected){
                        if (!gui.click(mousePosition)){
                            gui.selectAgent(nullptr);
                        }
                    }
                }
            }

            // Mouse drag
            else if (event.type == sf::Event::MouseMoved){
                auto oldMousePos = mousePosition;
                mousePosition = sf::Vector2<int>(event.mouseMove.x, event.mouseMove.y);
                if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)){
                    sf::Vector2f delta = sf::Vector2f(oldMousePos - mousePosition);
                    float speed = std::sqrt(delta.x*delta.x+delta.y*delta.y);
                    if (speed < thresholdSpeed || dragging)
                        camera.move(delta);
                }
                else {
                    gui.hover(mousePosition);
                }

                dragging = true;
                notMoving = false;
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

            if (notMoving)
                dragging = false;
        }

        if (config.shouldReload){
            printf("Reloading config\n");
            config.loadConfigFromFile(configFilename);
        }

        camera.update(dt.asSeconds());

        // Updating world
        if (!paused){
            world.update(dt.asSeconds()*timeFactor);
        }

        // Rendering
        window.clear(sf::Color::Black);
        world.draw(paused ? 0 : dt.asSeconds()*timeFactor);
        gui.draw(paused ? 0 : dt.asSeconds(), timeFactor);
        window.display();
    }

    return 0;
}