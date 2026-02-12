#include <iostream>
#include <vector>
#include <array>
#include <random>
#include "math.h"
#include "raylib.h"
#include "game_utils.h"


//TODO: Handle enemy decisions. And enemy doesn't have boosting animation yet.


static int SCREEN_WIDTH = 1920;
static int SCREEN_HEIGHT = 1080;

enum class ProjectileType {
    PLASMA,
    PROTON,
    VULCAN,
    COUNT
};

struct ProjectileFrames {
    std::vector<Texture2D> frames;
};

std::array<ProjectileFrames, (int)ProjectileType::COUNT> projectileAssets;

void loadProjectileFrames() {
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


std::vector<Projectile> playerProjectiles;
std::vector<Projectile> enemyProjectiles;


void updateActiveProjectiles() {
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
}


void drawActiveProjectiles() {
    for (Projectile& proj : playerProjectiles) {
        int width = projectileAssets[(int)proj.type].frames[0].width;
        int height = projectileAssets[(int)proj.type].frames[0].height;

        Vector2 projOrigin = {(float)(width / 2), (float)(height / 2)};

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


struct Player {
    Vector2 pos = { (float)(SCREEN_WIDTH/2), (float)(SCREEN_HEIGHT/2) };
    Vector2 velocity = { 0.f, 0.f };
    Texture2D texture;
    Camera2D camera = { 0 };

    Vector2 crosshairPos = {0.f, 0.f};

    std::vector<Texture2D> exhaustFrames;
    int exhaustFrameCount = 5; // Do not touch this unless you change the assets for exhaust.
    float frameTime = 0.1f;
    float timer = 0.0f;
    int frame = 0;

    float angle = 0.0f;
    float rotationSpeed = 3.0f;

    bool boosting = false;
    float speed = 2.0f;
    float maxSpeed = 400.f;
    float acceleration = 200.0f;

    ProjectileType mainGunAmmoType = ProjectileType::PLASMA;

    Player();
    void draw();
    void update();
    void move();
    void mainGunControl();
};


Player::Player() {
    texture = LoadTexture("../assets/player_b_m.png");

    exhaustFrames.resize(exhaustFrameCount);
    exhaustFrames[0] = LoadTexture("../assets/exhaust/exhaust_01.png");
    exhaustFrames[1] = LoadTexture("../assets/exhaust/exhaust_02.png");
    exhaustFrames[2] = LoadTexture("../assets/exhaust/exhaust_03.png");
    exhaustFrames[3] = LoadTexture("../assets/exhaust/exhaust_04.png");
    exhaustFrames[4] = LoadTexture("../assets/exhaust/exhaust_05.png");

    camera.zoom = 1.0f;
    camera.offset = {(float)SCREEN_WIDTH / 2.0f, (float)SCREEN_HEIGHT / 2.0f};
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

    float exhaustInset = 3.0f;
    float exhaustAlign = 2.0f;
    Vector2 exhaustLocal = { 0.0f - exhaustAlign, height/2 - exhaustInset }; // something something local space
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

    pos.x += velocity.x * dt;
    pos.y += velocity.y * dt;
}

void Player::mainGunControl() {
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

        playerProjectiles.push_back(newPlayerProj);
    }

    //Super scuffed but works.
}

void Player::update() {
    move();
    mainGunControl();

    float camera_x_target = pos.x;
    float camera_y_target = pos.y;

    camera.target = {camera_x_target, camera_y_target};

    crosshairPos = GetScreenToWorld2D(GetMousePosition(), camera);
}


struct Background {
    Texture2D texture;
    Vector2 offset = {0.f, 0.f};
    Vector2 lastCameraTarget = {0.f, 0.f};
    Vector2 scale;

    Background();
    void draw(Camera2D& playerCamera);
};

Background::Background() {
    texture = LoadTexture("../assets/background/blue-stars.png");

    scale.x = (float)SCREEN_WIDTH / texture.width;
    scale.y = (float)SCREEN_HEIGHT / texture.height;
};

void Background::draw(Camera2D& playerCamera) {
    Vector2 cameraDelta = {playerCamera.target.x - lastCameraTarget.x, playerCamera.target.y - lastCameraTarget.y};

    offset.x -= cameraDelta.x;
    offset.y -= cameraDelta.y;

    lastCameraTarget = playerCamera.target;
    
    offset.x = fmodf(offset.x, static_cast<float>(SCREEN_WIDTH));
    offset.y = fmodf(offset.y, static_cast<float>(SCREEN_HEIGHT));

    Rectangle src {
        0, 0,
        (float)texture.width,
        (float)texture.height
    };

    for (int y = -1; y <= 1; y++) {
        for (int x = -1; x <= 1; x++) {
            Rectangle dst {
                x * SCREEN_WIDTH + offset.x,
                y * SCREEN_HEIGHT + offset.y,
                texture.width * scale.x,
                texture.height * scale.y
            };

            DrawTexturePro(texture, src, dst, {0, 0}, 0, WHITE);
        }
    }
}


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
    Texture2D texture;
    Vector2 velocity;

    Decision action = Decision::STEADY;



    float angle = 0.f;
    float decisionAngle;
    float rotationSpeed = 6.0f;

    float speed = 4.0f;
    float acceleration = 200.f;
    float maxSpeed = 500.f;




    Enemy(Player& player, std::default_random_engine& enemyGenerator);
    void draw();
    void update(Player& player);
    void move(float dt);
    void chase(Vector2 playerPos, float dt);
    void fight(Vector2 playerPos, float dt);
    void decide(Player& player);
};
std::vector<Enemy> ActiveEnemies;


Enemy::Enemy(Player& player, std::default_random_engine& enemyGenerator) {
    Vector2 playerPos = player.pos;
    pos = rollRandomEnemySpawnPosition(playerPos, SCREEN_WIDTH, SCREEN_HEIGHT, enemyGenerator);
    std::cout << "Enemy generated at pos: " << (pos.x + SCREEN_WIDTH/2) << "," << (pos.y + SCREEN_HEIGHT/2) << "\n";

    texture = LoadTexture("../assets/enemy_2_r_m.png");
}

void Enemy::draw() {
    float width = static_cast<float>(texture.width);
    float height = static_cast<float>(texture.height);

    Rectangle src = { 0, 0, width, height }; // something something local space
    Rectangle dst = { pos.x, pos.y, width, height }; // something something world space
    Vector2 origin = { width/2, height/2 };


    DrawTexturePro(texture, src, dst, origin, angle * RAD2DEG - 90, WHITE);
}

void Enemy::update(Player& player) {
    decide(player);

    float dt = GetFrameTime();
    float damping = 0.98f;

    Vector2 playerPos = player.pos;


    switch(action) {
        case Decision::CHASE:
            chase(playerPos, dt);
            break;
        case Decision::STEADY:
            move(dt);
            break;
        default:
            velocity.x *= damping;
            velocity.y *= damping;
            break;
    }
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

void Enemy::fight(Vector2 playerPos, float dt) {
    float randomAngleOffset;
    float randomDistanceOffset;

    if (playerPos.x < pos.x && playerPos.y < pos.y)
    {

    }
    if (playerPos.x > pos.x && playerPos.y < pos.y)
    {

    }
    if (playerPos.x > pos.x && playerPos.y > pos.y)
    {

    }
    if (playerPos.x < pos.x && playerPos.y > pos.y)
    {

    }

    //TODO: Decide what to do here, probably decide a semi randomized angle from a deterministic direction. Meaning decide the direction/angle, then add random offset.
}


void Enemy::decide(Player& player) {
    Vector2 playerPos = player.pos;
    int chaseRange = 1000;

    if (getDistance(pos, playerPos) > chaseRange)
    {
        action = Decision::CHASE;
        return;
    }
    else
    {
        action = Decision::FIGHT;
        return;
    }


    action = Decision::STEADY;

    //TODO: setup a system that is semi randomized movement around the player.
}


int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Triangle");
    SetTargetFPS(165);

    Player player;
    Background background;

    auto enemyGenerator = initEnemyGenerator();
    loadProjectileFrames();

    Enemy enemy(player, enemyGenerator);

    while (!WindowShouldClose()) {

        player.update();
        enemy.update(player);

        updateActiveProjectiles();

        BeginDrawing();

            ClearBackground(BLACK);

                background.draw(player.camera);

                    BeginMode2D(player.camera);
                        player.draw();
                        enemy.draw();
                        drawActiveProjectiles();
                    EndMode2D();

        EndDrawing();
    }

    CloseWindow();
}