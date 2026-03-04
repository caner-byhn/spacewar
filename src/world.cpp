#include "world.hpp"
#include "enemy.hpp"
#include "raylib.h"
#include <iostream>
#include <random>

Projectile::Projectile(ProjectileType type, float angle) : type(type), angle(angle){
    if (type == ProjectileType::PLASMA) {
        speed = 2000;
        maxDistanceTraveled = 4000;
    }
    if (type == ProjectileType::VULCAN) {
        speed = 1000;
        maxDistanceTraveled = 8000;
    }
    if (type == ProjectileType::PROTON) {
        speed = 400;
        maxDistanceTraveled = 2000;
    }
}


Background::Background() {
    texture = LoadTexture("../assets/background/blue-stars.png");

    scale.x = (float)GetScreenWidth() / texture.width;
    scale.y = (float)GetScreenHeight() / texture.height;
};


void Background::draw(Camera2D& playerCamera) {
    Vector2 cameraDelta = {playerCamera.target.x - lastCameraTarget.x, playerCamera.target.y - lastCameraTarget.y};

    offset.x -= cameraDelta.x;
    offset.y -= cameraDelta.y;

    lastCameraTarget = playerCamera.target;

    offset.x = fmodf(offset.x, static_cast<float>(GetScreenWidth()));
    offset.y = fmodf(offset.y, static_cast<float>(GetScreenHeight()));

    Rectangle src {
        0, 0,
        (float)texture.width,
        (float)texture.height
    };

    for (int y = -1; y <= 1; y++) {
        for (int x = -1; x <= 1; x++) {
            Rectangle dst {
                x * GetScreenWidth() + offset.x,
                y * GetScreenHeight() + offset.y,
                texture.width * scale.x,
                texture.height * scale.y
            };

            DrawTexturePro(texture, src, dst, {0, 0}, 0, WHITE);
        }
    }
}


World::World() {
    spawner.time = 0.f;
    spawner.timer = 1.5f;
    spawner.activeSpawns = 0;
    spawner.limit = 3;

    loadPlayerExhaustFrames();
    loadExplosionFrames();
    loadProjectileFrames();
    loadEnemyTextures();
}

void World::loadPlayerExhaustFrames() {
    playerExhaust.push_back(LoadTexture("../assets/exhaust/exhaust_01.png"));
    playerExhaust.push_back(LoadTexture("../assets/exhaust/exhaust_02.png"));
    playerExhaust.push_back(LoadTexture("../assets/exhaust/exhaust_03.png"));
    playerExhaust.push_back(LoadTexture("../assets/exhaust/exhaust_04.png"));
    playerExhaust.push_back(LoadTexture("../assets/exhaust/exhaust_05.png"));
}

void World::loadExplosionFrames() {
    explosion.push_back(LoadTexture("../assets/explosions/explosion_1_01.png"));
    explosion.push_back(LoadTexture("../assets/explosions/explosion_1_02.png"));
    explosion.push_back(LoadTexture("../assets/explosions/explosion_1_03.png"));
    explosion.push_back(LoadTexture("../assets/explosions/explosion_1_04.png"));
    explosion.push_back(LoadTexture("../assets/explosions/explosion_1_05.png"));
    explosion.push_back(LoadTexture("../assets/explosions/explosion_1_06.png"));
    explosion.push_back(LoadTexture("../assets/explosions/explosion_1_07.png"));
    explosion.push_back(LoadTexture("../assets/explosions/explosion_1_08.png"));
    explosion.push_back(LoadTexture("../assets/explosions/explosion_1_09.png"));
    explosion.push_back(LoadTexture("../assets/explosions/explosion_1_10.png"));
    explosion.push_back(LoadTexture("../assets/explosions/explosion_1_11.png"));
}

void World::loadProjectileFrames() {
    projectileAssets[(int)ProjectileType::PLASMA].frames = {
        LoadTexture("../assets/projectiles/plasma_1.png"),
        LoadTexture("../assets/projectiles/plasma_2.png")
    };
    projectileAssets[(int)ProjectileType::PROTON].frames = {
        LoadTexture("../assets/projectiles/proton_01.png"),
        LoadTexture("../assets/projectiles/proton_02.png"),
        LoadTexture("../assets/projectiles/proton_03.png")
    };
    projectileAssets[(int)ProjectileType::VULCAN].frames = {
        LoadTexture("../assets/projectiles/vulcan_1.png"),
        LoadTexture("../assets/projectiles/vulcan_2.png"),
        LoadTexture("../assets/projectiles/vulcan_3.png")
    };
}

void World::loadEnemyTextures() {
    enemyTex = LoadTexture("../assets/enemy_2_r_m.png");
}

void World::spawnEnemy(Player& player, std::default_random_engine& enemyGenerator) {
    static std::uniform_real_distribution<float> spawnDist(1.f, 2.f);

    spawner.time += GetFrameTime();
    if (ActiveEnemies.size() < spawner.limit) {
        if (spawner.time > spawner.timer) {
            Enemy newEnemy(player, enemyGenerator, *this);
            ActiveEnemies.push_back(newEnemy);
            spawner.timer = spawnDist(enemyGenerator);
            spawner.time = 0.f;
        }
    }
}

void World::updateActiveEnemies(Player& player, std::default_random_engine& generator) {
    for (auto it = ActiveEnemies.begin(); it != ActiveEnemies.end(); ) {
        if (it->readyToRemove) {
            it = ActiveEnemies.erase(it);
        }
        else {
            it->update(player, generator, *this);
            ++it;
        }
    }
}

void World::drawActiveEnemies() {
    for(auto& en : ActiveEnemies) {
        en.draw();
    }
}

void World::updateActiveProjectiles() {
    float dt = GetFrameTime();

    for (auto it = playerProjectiles.begin(); it != playerProjectiles.end(); ) {
        it->pos.x += it->velocity.x * dt;
        it->pos.y += it->velocity.y * dt;

        it->distanceTraveled += it->speed * dt;

        if (it->distanceTraveled >= it->maxDistanceTraveled) {
            std::cout << "Projectile destroyed at pos: " << it->pos.x << "," << it->pos.y << "\n";
            it = playerProjectiles.erase(it);
        }
        else {
            ++it;
        }
    }

    for (auto it = enemyProjectiles.begin(); it != enemyProjectiles.end(); ) {
        it->pos.x += it->velocity.x * dt;
        it->pos.y += it->velocity.y * dt;

        it->distanceTraveled += it->speed * dt;

        if (it->distanceTraveled >= it->maxDistanceTraveled) {
            std::cout << "Projectile reached maximum distance at pos: " << it->pos.x << "," << it->pos.y << "\n";
            it = enemyProjectiles.erase(it);
        }
        else {
            ++it;
        }
    }
}

void World::drawActiveProjectiles() {
    for (Projectile& proj : playerProjectiles) {
        int width = projectileAssets[(int)proj.type].frames[0].width;
        int height = projectileAssets[(int)proj.type].frames[0].height;

        Vector2 projOrigin = {(float)width / 2, (float)height / 2};

        Rectangle src = {0, 0, (float)width, (float)height};
        Rectangle dest = {proj.pos.x, proj.pos.y, (float)width, (float)height};

        proj.frameTimer += GetFrameTime();
        if (proj.frameTimer > proj.frameTime) {
            proj.frameTimer = 0.0f;
            proj.frame = (proj.frame + 1) % projectileAssets[(int)proj.type].frames.size();
        }

        DrawTexturePro(projectileAssets[(int)proj.type].frames[proj.frame], src, dest, projOrigin, proj.angle * RAD2DEG, WHITE);
    }

    for (Projectile& proj : enemyProjectiles) {
        int width = projectileAssets[(int)proj.type].frames[0].width;
        int height = projectileAssets[(int)proj.type].frames[0].height;

        Vector2 projOrigin = {(float)width / 2, (float)height / 2};

        Rectangle src = {0, 0, (float)width, (float)height};
        Rectangle dest = {proj.pos.x, proj.pos.y, (float)width, (float)height};

        proj.frameTimer += GetFrameTime();
        if (proj.frameTimer > proj.frameTime) {
            proj.frameTimer = 0.0f;
            proj.frame = (proj.frame + 1) % projectileAssets[(int)proj.type].frames.size();
        }

        DrawTexturePro(projectileAssets[(int)proj.type].frames[proj.frame], src, dest, projOrigin, proj.angle * RAD2DEG, WHITE);
    }
}
