#pragma once

#include <vector>
#include "raylib.h"


struct World;
enum class ProjectileType;


struct Player {
    Vector2 pos = { (float)GetScreenWidth() / 2, (float)GetScreenHeight() / 2 };
    Vector2 velocity = { 0.f, 0.f };
    Texture2D texture;
    Camera2D camera = { 0 };

    Vector2 crosshairPos = {0.f, 0.f};

    std::vector<Texture2D>& exhaustFrames;
    int exhaustFrameCount; // Do not touch this unless you change the assets for exhaust.
    float frameTime = 0.1f;
    float timer = 0.0f;
    int frame = 0;

    float angle = 0.0f;
    float rotationSpeed = 3.0f;

    bool boosting = false;
    float speed = 2.0f;
    float maxSpeed = 400.f;
    float acceleration = 200.0f;

    ProjectileType mainGunAmmoType;

    Player(World &world);
    void draw();
    void update(World &world);
    void move();
    void mainGunControl(World &world);
};
