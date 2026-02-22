#pragma once

#include "world.hpp"
#include "player.hpp"
#include <random>


struct Game {
    World world;
    Background background;
    Player player;
    std::default_random_engine randomGenerator;

    Game();
    void update();
    void drawBackground();
    void draw();
};
