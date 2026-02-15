#include <iostream>
#include <math.h>
#include <random>
#include "raylib.h"



inline Vector2 findTriangleCenter(Vector2 t1, Vector2 t2, Vector2 t3) {
    Vector2 center;
    center.x = (t1.x, t2.x, t3.x) / 3.0f;
    center.y = (t1.y, t2.y, t3.y) / 3.0f;
    return center;
}

inline Vector2 rotatePoint(Vector2 p, float angle){
    Vector2 rotate;

    float c = cosf(angle);
    float s = sinf(angle);

    rotate.x = p.x * c - p.y * s;
    rotate.y = p.x * s + p.y * c;

    return rotate;
}

inline float getDistance(Vector2 v1, Vector2 v2) {
    float dx = v1.x - v2.x;
    float dy = v1.y - v2.y;

    return sqrtf(dx*dx + dy*dy);
}

inline std::default_random_engine initEnemyGenerator() {
    return std::default_random_engine{ std::random_device{}() };
}

inline Vector2 rollRandomEnemySpawnPosition(Vector2 current_player_world_pos, int screenWidth, int screenHeight, std::default_random_engine& generator) {
    float spawn_pos_offset = 100;

    std::uniform_int_distribution<int> sideDist(1,4);
    int side = sideDist(generator);

    std::uniform_int_distribution<int> heightDist(-(screenHeight / 2), (screenHeight / 2));
    int randomHeight = heightDist(generator);

    std::uniform_int_distribution<int> widthDist(-(screenWidth / 2), (screenWidth / 2));
    int randomWidth = widthDist(generator);

    float world_spawn_x;
    float world_spawn_y;

    std::cout << "Enemy spawn at side: " << side << "\n";

    if (side == 1) {
        world_spawn_x = current_player_world_pos.x - (screenWidth / 2) - spawn_pos_offset;
        world_spawn_y = randomHeight + current_player_world_pos.y;
    }
    else if (side == 2) {
        world_spawn_x = randomWidth + current_player_world_pos.x;
        world_spawn_y = current_player_world_pos.y - (screenHeight / 2) - spawn_pos_offset;
    }
    else if (side == 3) {
        world_spawn_x = current_player_world_pos.x + (screenWidth / 2) + spawn_pos_offset;
        world_spawn_y = randomHeight + current_player_world_pos.y;
    }
    else if (side == 4) {
        world_spawn_x = randomWidth + current_player_world_pos.x;
        world_spawn_y = current_player_world_pos.y + (screenHeight / 2) + spawn_pos_offset;
    }

    Vector2 enemy_spawn_pos = {world_spawn_x, world_spawn_y};
    return enemy_spawn_pos;
}


inline Vector2 normalize(Vector2 vec) {
    float len = sqrtf(vec.x * vec.x + vec.y * vec.y);
    if (len > 0.0f)
    {
        vec.x /= len;
        vec.y /= len;
    }

    return vec;
}


//TODO: maybe abstract movement calculations here it is starting to get confusing.
