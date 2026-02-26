#include "world.hpp"
#include "enemy.hpp"
#include "raylib.h"
#include <iostream>

Projectile::Projectile(ProjectileType type, float angle) : type(type), angle(angle){
    if (type == ProjectileType::PLASMA) {
        speed = 700;
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
    loadPlayerExhaustFrames();
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
    Enemy newEnemy(player, enemyGenerator, *this);
    ActiveEnemies.push_back(newEnemy);
}

void World::updateActiveEnemies(Player& player, std::default_random_engine& generator) {
    for(auto& en : ActiveEnemies) {
        en.update(player, generator, *this);
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
            std::cout << "Projectile destroyed at pos: " << it->pos.x << "," << it->pos.y << "\n";
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
