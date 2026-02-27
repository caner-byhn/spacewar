#pragma once

#include "raylib.h"
#include "player.hpp"
#include "SAT.hpp"
#include <random>

struct World;

enum class Enemies {
    //TODO: Eeventually use enemy types.
};


enum class Decision {
    CHASE,
    STEADY,
    FIGHT
};


struct Enemy {
    Vector2 pos;
    Texture2D& texture;
    Vector2 velocity;

    SAT sat;
    bool isColliding = false;

    std::vector<Texture2D>& exhaustFrames;
    int exhaustFrameCount; // Do not touch this unless you change the assets for exhaust.
    float frameTime = 0.1f;
    float timer = 0.0f;
    int frame = 0;

    Decision action = Decision::STEADY;
    bool fighting = false;
    float fightDecisionTimer = 0.0f;
    float resetFightDirTime = 1.f;

    float angle = 0.f;
    float decisionAngle;
    float rotationSpeed = 6.0f;

    float speed = 4.0f;
    float acceleration = 200.f;
    float maxSpeed = 500.f;

    bool firing = false;
    float burstTimeCounter = 0.f;
    float burstInterval = 0.5f;
    float burstCooldown = 1.5f;
    unsigned int shotsCount = 0;
    unsigned int maxShots = 3;

    Enemy(Player& player, std::default_random_engine& enemyGenerator, World& world);
    void draw();
    void update(Player& player, std::default_random_engine& generator, World& world);
    void move(float dt);

    void chase(Vector2 playerPos, float dt);
    void fight(Vector2 playerPos, float dt, std::default_random_engine& generator);
    void decide(Player& player);

    void burstFire(Player& player, World& world);
};
