#include "enemy.hpp"
#include "SAT.hpp"
#include "player.hpp"
#include "raylib.h"
#include "game_utils.hpp"
#include "world.hpp"
#include <iostream>


Enemy::Enemy(Player& player, std::default_random_engine& enemyGenerator, World& world) : texture(world.enemyTex), exhaustFrames(world.playerExhaust), exhaustFrameCount(world.playerExhaust.size()) {
    Vector2 playerPos = player.pos;
    pos = rollRandomEnemySpawnPosition(playerPos, enemyGenerator);
    std::cout << "Enemy generated at pos: " << (pos.x + (float)GetScreenWidth()/2) << "," << (pos.y + (float)GetScreenHeight()/2) << "\n";

    Vector2 halfSize = {static_cast<float>(texture.width) / 4,static_cast<float>(texture.height) / 4};
    sat.origin = pos;
    sat.halfSize = halfSize;
}


void Enemy::draw() {
    float width = static_cast<float>(texture.width);
    float height = static_cast<float>(texture.height);

    float exhaust_width = static_cast<float>(exhaustFrames[0].width);
    float exhaust_height = static_cast<float>(exhaustFrames[0].height);
    float exhaust_offset = (width - exhaust_width) / 2;

    Rectangle src = { 0, 0, width, height }; // something something local space
    Rectangle dst = { pos.x, pos.y, width, height }; // something something world space
    Vector2 origin = { width/2, height/2 };

    float adjustedAngle = angle + PI/2;

    float exhaustInsert = 3.0f;
    float exhaustAlign = 1.0f;
    Vector2 exhaustLocal = { 0.0f - exhaustAlign, height/2 - exhaustInsert }; // something something local space
    Vector2 exhaustOffsetWorld = { exhaustLocal.x * cosf(adjustedAngle) - exhaustLocal.y * sinf(adjustedAngle), exhaustLocal.x * sinf(adjustedAngle) + exhaustLocal.y * cosf(adjustedAngle) }; // rotate the offset in the local(?) space

    Rectangle exhaust_src = { 0, 0, exhaust_width, exhaust_height };
    Rectangle exhaust_dst = { pos.x + exhaustOffsetWorld.x, pos.y + exhaustOffsetWorld.y, exhaust_width, exhaust_height };
    Vector2 exhaust_origin = { exhaust_width/2, exhaust_height/2 };

    timer += GetFrameTime();
    if(timer > frameTime) {
        timer = 0.0f;
        frame = (frame + 1) % exhaustFrameCount;
    }

    Color colColor = GREEN;

    if (isColliding) {
        colColor = RED;
    }

    DrawTexturePro(texture, src, dst, origin, angle * RAD2DEG - 90, WHITE);
    DrawTexturePro(exhaustFrames[frame], exhaust_src, exhaust_dst, exhaust_origin, angle * RAD2DEG + 90, YELLOW);
    drawSATdebugOutline(width/2, height/2, pos, angle, colColor);
}


void Enemy::update(Player& player, std::default_random_engine& generator, World& world) {
    decide(player);

    sat.origin = pos;
    updateSATAxisRotation(sat, angle);

    float dt = GetFrameTime();
    float damping = 0.98f;

    Vector2 playerPos = player.pos;

    fightDecisionTimer += dt;
    if(fightDecisionTimer > resetFightDirTime) {
        static std::uniform_real_distribution<float> randTime(0.5f,1.5f);
        fighting = false;
        fightDecisionTimer = 0.f;
        resetFightDirTime = randTime(generator);
    }


    switch(action) {
        case Decision::CHASE:
            chase(playerPos, dt);
            break;
        case Decision::FIGHT:
            fight(playerPos, dt, generator);
            break;
        case Decision::STEADY:
            move(dt);
            break;
        default:
            velocity.x *= damping;
            velocity.y *= damping;
            break;
    }
    burstFire(player, world);
}


void Enemy::move(float dt) {
    Vector2 forward = {cosf(angle), sinf(angle)};
    Vector2 right = {forward.y, -forward.x};

    velocity.x += forward.x * acceleration * dt;
    velocity.y += forward.y * acceleration * dt;

    float currentSpeed = sqrtf(velocity.x * velocity.x + velocity.y * velocity.y);

    if(currentSpeed > maxSpeed) {
        velocity.x = (velocity.x / currentSpeed) * maxSpeed;
        velocity.y = (velocity.y / currentSpeed) * maxSpeed;
    }

    float forwardSpeed = velocity.x * forward.x + velocity.y * forward.y;
    float sideSpeed    = velocity.x * right.x   + velocity.y * right.y;

    sideSpeed *= 0.2f;

    velocity.x = forward.x * forwardSpeed + right.x * sideSpeed;
    velocity.y = forward.y * forwardSpeed + right.x * sideSpeed;

    pos.x += velocity.x * dt;
    pos.y += velocity.y * dt;
}


void Enemy::chase(Vector2 playerPos, float dt) {
    Vector2 posDiff = {playerPos.x - pos.x, playerPos.y - pos.y};
    Vector2 targetForward = normalize(posDiff);
    float targetAngle = atan2f(targetForward.y, targetForward.x);

    float delta = targetAngle - angle;
    delta = atan2f(sinf(delta), cosf(delta));

    float step = rotationSpeed * dt;

    if (fabs(delta) <= step)
    {
        angle = targetAngle;
    }
    else
    {
        angle += step * (delta > 0 ? 1.0f : -1.0f);
    }

    move(dt);
}


void Enemy::fight(Vector2 playerPos, float dt, std::default_random_engine& generator) {
    static std::uniform_real_distribution<float> randomAngle(static_cast<float>((-30 * (PI/180))), static_cast<float>((30 * (PI/180))));
    static std::uniform_int_distribution<int> randomOffset(-75,75);
    static std::uniform_int_distribution<int> randomDirection(1,2);

    float randomAngleOffset;
    float randomDistanceOffset;
    float randomDir;

    if(!fighting) {
        randomAngleOffset = randomAngle(generator);
        randomDistanceOffset = randomOffset(generator);
        randomDir = randomDirection(generator);

        //top left
        if (playerPos.x < pos.x - randomDistanceOffset && playerPos.y < pos.y - randomDistanceOffset)
        {
            if(randomDir == 1)
                decisionAngle = (3*PI)/2 + randomAngleOffset;
            else
                decisionAngle = PI + randomAngleOffset;
        }
        //top right
        if (playerPos.x > pos.x + randomDistanceOffset && playerPos.y < pos.y - randomDistanceOffset)
        {
            if(randomDir == 1)
                decisionAngle = 0.f + randomAngleOffset;
            else
                decisionAngle = (3*PI)/2 + randomAngleOffset;
        }
        //bottom right
        if (playerPos.x > pos.x + randomDistanceOffset && playerPos.y > pos.y + randomDistanceOffset)
        {
            if(randomDir == 1)
                decisionAngle = 0.f + randomAngleOffset;
            else
                decisionAngle = PI/2 + randomAngleOffset;
        }
        //bottom left
        if (playerPos.x < pos.x - randomDistanceOffset && playerPos.y > pos.y + randomDistanceOffset)
        {
            if(randomDir == 1)
                decisionAngle = PI/2 + randomAngleOffset;
            else
                decisionAngle = PI + randomAngleOffset;
        }

        fighting = true;
    }

    float step = rotationSpeed * dt;
    float delta = decisionAngle - angle;
    delta = atan2f(sinf(delta), cosf(delta));

    if (fabs(delta) <= step)
    {
        angle = decisionAngle;
    }
    else
    {
        angle += step * (delta > 0 ? 1.0f : -1.0f);
    }

    move(dt);
}


void Enemy::decide(Player& player) {
    Vector2 playerPos = player.pos;
    int chaseRange = 1000;

    if (getDistance(pos, playerPos) > chaseRange)
    {
        action = Decision::CHASE;
        fighting = false;
        return;
    }
    else {
        action = Decision::FIGHT;
        return;
    }

    action = Decision::STEADY;
    fighting = false;
    //TODO: setup a system that is semi randomized movement around the player.
}


void Enemy::burstFire(Player& player, World& world) {
    Vector2 posDiff = {player.pos.x + player.lastSpeed.x - pos.x, player.pos.y + player.lastSpeed.y - pos.y};
    Vector2 targetDir = normalize(posDiff);
    float dirAngle = atan2(targetDir.y, targetDir.x) + PI/2;

    burstTimeCounter += GetFrameTime();
    if (!firing) {
        if (burstTimeCounter >= burstCooldown) {
            firing = true;
            burstTimeCounter = 0.f;
        }
    }
    else {
        if (burstTimeCounter >= burstInterval && shotsCount < maxShots) {
            Projectile newProj(ProjectileType::VULCAN, dirAngle);
            newProj.pos = pos;
            newProj.velocity = {targetDir.x * newProj.speed, targetDir.y * newProj.speed};
            world.enemyProjectiles.push_back(newProj);
            shotsCount++;
            burstTimeCounter = 0.f;
        }
        else if (shotsCount >= maxShots) {
            shotsCount = 0;
            burstTimeCounter = 0.f;
            firing = false;
        }
    }
}

void Enemy::checkCollision(World& world) {
    for (auto it = world.playerProjectiles.begin(); it != world.playerProjectiles.end(); ) {
        if (SATvsPoint(sat, it->pos)) {
            isColliding = true;
            std::cout << "Player projectile hit and destroyed at pos: " << it->pos.x << "," << it->pos.y << "\n";
            it = world.playerProjectiles.erase(it);
        }
        else {
            ++it;
        }
    }
}
