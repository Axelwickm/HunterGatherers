# HunterGatherers
Artificial life simulation of agents in a 2d-world. Made for an AI-course.
Written in C++17, with graphics done in SFML.

It features humanoid agents in a 2D world, where they sustain themselves on food, reproduce asexually, and engage in various interactions.

Report: [HunterGatherers.pdf](HunterGatherers.pdf)

Timelapse: https://www.youtube.com/watch?v=fuLsRUkQlHc


## Installation

Clone repo:
1. Clone repo: `git clone --recurse-submodules https://github.com/Axelwickm/HunterGatherers.git`
2. Build Docker Image: `docker build -t huntergatherers .`
3. Run: `docker run -it --rm --name hunter_gatherers \
    -e DISPLAY=$DISPLAY \
    -v /tmp/.X11-unix:/tmp/.X11-unix \
    --device /dev/dri:/dev/dri \
    --gpus all \
    huntergatherers`

## Info

The HunterGatherers project is an engaging artificial life simulation set in a dynamic 2D world. It showcases humanoid agents with autonomous behaviors, including foraging for food, reproducing, and responding intelligently to environmental changes. The simulation's core lies in its AI-driven agents that demonstrate complex interactions and survival strategies. Key features include real-time world rendering, detailed statistical analysis tools, and a user-friendly graphical interface. The project stands out for its customizability, allowing users to tailor numerous simulation parameters via `Config.json`, offering a versatile platform for experimentation in emergent system dynamics.



Note: the current dockerfile doesn't support GPU OpenCl. If you get this working, or build ourside of the container, the performance will be better.

