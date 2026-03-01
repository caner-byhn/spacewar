#pragma once

#include <vector>
#include "raylib.h"
#include "SAT.hpp"


struct World;
enum class ProjectileType;

struct FrameTimer {
    float frameTime;
    float timer;
    int frame;
};

enum class PlayerState {
    ALIVE,
    DEAD
};

struct Player {
    Vector2 pos = { (float)GetScreenWidth() / 2, (float)GetScreenHeight() / 2 };
    Vector2 velocity = { 0.f, 0.f };
    Texture2D texture;
    Camera2D camera = { 0 };

    SAT sat;
    bool isColliding = false;
    PlayerState playerState = PlayerState::ALIVE;

    Vector2 crosshairPos = {0.f, 0.f};

    std::vector<Texture2D>& explosions;
    int explosionFrameCount;
    FrameTimer explodeTimer = {0.1f, 0.0f, 0};

    std::vector<Texture2D>& exhaustFrames;
    int exhaustFrameCount; // Do not touch this unless you change the assets for exhaust.
    FrameTimer exhaustTimer = {0.1f, 0.0f, 0};

    float angle = 0.0f;
    float rotationSpeed = 3.0f;

    bool boosting = false;
    float speed = 2.0f;
    float maxSpeed = 400.f;
    float acceleration = 200.0f;
    Vector2 lastSpeed;

    ProjectileType mainGunAmmoType;

    Player(World &world);
    void draw();
    void update(World &world);
    void move();
    void mainGunControl(World &world);
    void checkCollision(World &world);
};
