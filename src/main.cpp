#include "game.hpp"
#include "raylib.h"


// TODO: Make the enemy fight AI rethink in random intervals rather than a set timer. This is easily doable.
// And the fighting AI might need optimizations as some of the operations there can cause problems with
// large amounts of enemies.


static int SCREEN_WIDTH = 1920;
static int SCREEN_HEIGHT = 1080;



int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Triangle");
    SetTargetFPS(165);

    Game game;

    while (!WindowShouldClose()) {
        game.update();

        BeginDrawing();
            ClearBackground(BLACK);
                game.drawBackground();

                    BeginMode2D(game.player.camera);
                        game.draw();
                    EndMode2D();

        EndDrawing();
    }

    CloseWindow();
}
