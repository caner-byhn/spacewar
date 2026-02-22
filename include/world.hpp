#pragma once

#include "enemy.hpp"
#include "raylib.h"
#include <random>
#include <vector>
#include <array>


struct Player;

enum class ProjectileType {
    PLASMA,
    PROTON,
    VULCAN,
    COUNT
};

struct ProjectileFrames {
    std::vector<Texture2D> frames;
};

struct Projectile {
    ProjectileType type;
    Vector2 pos;
    Vector2 velocity;

    int frame = 0;
    float frameTimer = 0.0f;
    float frameTime = 0.1f;

    float angle;
    float speed;
    float distanceTraveled = 0.0f;
    float maxDistanceTraveled;

    Projectile(ProjectileType type, float angle);
};

struct Background {
    Texture2D texture;
    Vector2 offset = {0.f, 0.f};
    Vector2 lastCameraTarget = {0.f, 0.f};
    Vector2 scale;

    Background();
    void draw(Camera2D& playerCamera);
};

struct World {
    std::array<ProjectileFrames, (int)ProjectileType::COUNT> projectileAssets;
    std::vector<Texture2D> playerExhaust;

    Texture2D enemyTex; //this is a placeholder
    std::vector<Enemy> ActiveEnemies;

    std::vector<Projectile> playerProjectiles;
    std::vector<Projectile> enemyProjectiles;


    World();
    void loadPlayerExhaustFrames();
    void loadProjectileFrames();
    void loadEnemyTextures();

    void spawnEnemy(Player& player, std::default_random_engine& enemyGenerator);

    void updateActiveEnemies(Player& player, std::default_random_engine& generator);
    void drawActiveEnemies();
    void updateActiveProjectiles();
    void drawActiveProjectiles();
};
