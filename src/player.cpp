#include "world.hpp"
#include "player.hpp"
#include "SAT.hpp"
#include "game_utils.hpp"
#include "raylib.h"
#include <vector>


Player::Player(World &world) : exhaustFrames(world.playerExhaust), exhaustFrameCount(world.playerExhaust.size()), mainGunAmmoType(ProjectileType::PLASMA) {
    texture = LoadTexture("../assets/player_b_m.png");
    camera.zoom = 1.0f;
    camera.offset = {(float)GetScreenWidth() / 2.0f, (float)GetScreenHeight() / 2.0f};

    Vector2 halfSize = {static_cast<float>(texture.width) / 4, static_cast<float>(texture.height) / 4};
    sat.halfSize = halfSize;
    sat.origin = pos;
}


//I don't even know what the fuck is this function calculating at this point.
void Player::draw() {
    float width = static_cast<float>(texture.width);
    float height = static_cast<float>(texture.height);

    float exhaust_width = static_cast<float>(exhaustFrames[0].width);
    float exhaust_height = static_cast<float>(exhaustFrames[0].height);
    float exhaust_offset = (width - exhaust_width) / 2;

    Rectangle src = { 0, 0, width, height }; // something something local space
    Rectangle dst = { pos.x, pos.y, width, height }; // something something world space
    Vector2 origin = { width/2, height/2 };

    float exhaustInsert = 3.0f;
    float exhaustAlign = 2.0f;
    Vector2 exhaustLocal = { 0.0f - exhaustAlign, height/2 - exhaustInsert }; // something something local space
    Vector2 exhaustOffsetWorld = { exhaustLocal.x * cosf(angle) - exhaustLocal.y * sinf(angle), exhaustLocal.x * sinf(angle) + exhaustLocal.y * cosf(angle) }; // rotate the offset in the local(?) space

    Rectangle exhaust_src = { 0, 0, exhaust_width, exhaust_height };
    Rectangle exhaust_dst = { pos.x + exhaustOffsetWorld.x, pos.y + exhaustOffsetWorld.y, exhaust_width, exhaust_height };
    Vector2 exhaust_origin = { exhaust_width/2, exhaust_height/2 };

    timer += GetFrameTime();
    if(timer > frameTime) {
        timer = 0.0f;
        frame = (frame + 1) % exhaustFrameCount;
    }

    DrawTexturePro(texture, src, dst, origin, angle * RAD2DEG, WHITE);

    DrawLineEx(pos, {pos.x, pos.y-10.f}, 1.f, RED);

    //Draw collision box for debug.
    Color colColor = GREEN;

    if (isColliding) {
        colColor = RED;
    }

    drawSATdebugOutline(width/2, height/2, pos, angle, colColor);


    if (boosting) {
        DrawTexturePro(exhaustFrames[frame], exhaust_src, exhaust_dst, exhaust_origin, angle * RAD2DEG, WHITE);
    }

    DrawCircleLines(crosshairPos.x, crosshairPos.y, 3.f, WHITE);
}


void Player::move() {
    float dt = GetFrameTime();
    float damping = 0.98f;

    Vector2 forward = {sinf(angle), -cosf(angle)};
    Vector2 right = {forward.y, -forward.x};

    if(IsKeyDown(KEY_A)) {
        angle -= rotationSpeed * dt;
    }
    if(IsKeyDown(KEY_D)) {
        angle += rotationSpeed * dt;
    }

    if(IsKeyDown(KEY_W)) {
        boosting = true;
        velocity.x += forward.x * acceleration * dt;
        velocity.y += forward.y * acceleration * dt;
    }
    else {
        boosting = false;
        velocity.x *= damping;
        velocity.y *= damping;
    }

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

    lastSpeed = {velocity.x * dt, velocity.y * dt};

    pos.x += velocity.x * dt;
    pos.y += velocity.y * dt;
}


void Player::mainGunControl(World& world) {
    float width = static_cast<float>(texture.width);
    float height = static_cast<float>(texture.height);

    Vector2 playerOrigin = {width / 2, height / 2};
    Vector2 muzzleLocal = { 0.f, -height / 2};
    Vector2 rotated = rotatePoint(muzzleLocal, angle);
    Vector2 muzzleWorld = { pos.x + rotated.x, pos.y + rotated.y };

    Vector2 forward = {sinf(angle), -cosf(angle)};

    if(IsKeyPressed(KEY_ONE)) mainGunAmmoType = ProjectileType::PLASMA;
    if(IsKeyPressed(KEY_TWO)) mainGunAmmoType = ProjectileType::VULCAN;
    if(IsKeyPressed(KEY_THREE)) mainGunAmmoType = ProjectileType::PROTON;

    if(IsKeyPressed(KEY_F)) {
        Projectile newPlayerProj(mainGunAmmoType, angle);

        newPlayerProj.pos = muzzleWorld;
        newPlayerProj.velocity = {forward.x * newPlayerProj.speed, forward.y * newPlayerProj.speed};

        std::cout << "Projectile fired at pos: " << muzzleWorld.x << "," << muzzleWorld.y << "\n";

        world.playerProjectiles.push_back(newPlayerProj);
    }

    //Super scuffed but works.
}


void Player::update(World& world) {
    move();
    mainGunControl(world);

    sat.origin = pos;
    updateSATAxisRotation(sat, angle);

    checkCollision(world);

    float camera_x_target = pos.x;
    float camera_y_target = pos.y;

    camera.target = {camera_x_target, camera_y_target};

    crosshairPos = GetScreenToWorld2D(GetMousePosition(), camera);
}


void Player::checkCollision(World& world) {
    isColliding = false;
    for (auto& e : world.ActiveEnemies) {
        if (SATvsSAT(sat, e.sat)) {
            isColliding = true;
            e.isColliding = true;
        }
        else {
            e.isColliding = false;
        }
    }

    for (auto it = world.enemyProjectiles.begin(); it != world.enemyProjectiles.end(); ) {
        if (SATvsPoint(sat, it->pos)) {
            isColliding = true;
            std::cout << "Enemy projectile hit and destroyed at pos: " << it->pos.x << "," << it->pos.y << "\n";
            it = world.enemyProjectiles.erase(it);
        }
        else {
            ++it;
        }
    }
}
