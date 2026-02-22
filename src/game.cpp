#include "game.hpp"
#include "world.hpp"
#include "player.hpp"
#include "game_utils.hpp"


Game::Game(): player(world) {
    randomGenerator = initEnemyGenerator();
    world.spawnEnemy(player, randomGenerator);
}

void Game::update() {
    player.update(world);
    world.updateActiveEnemies(player, randomGenerator);
    world.updateActiveProjectiles();
}

void Game::drawBackground() {
    background.draw(player.camera);
}

void Game::draw() {
    player.draw();
    world.drawActiveEnemies();
    world.drawActiveProjectiles();
}
