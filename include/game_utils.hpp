#pragma once

#include <iostream>
#include <math.h>
#include <random>
#include <vector>
#include "raylib.h"


inline Vector2 rotatePoint(Vector2 p, float angle){
    Vector2 rotate;

    float c = cosf(angle);
    float s = sinf(angle);

    rotate.x = p.x * c - p.y * s;
    rotate.y = p.x * s + p.y * c;

    return rotate;
}

inline float dot(Vector2 v1, Vector2 v2) {
    return v1.x*v2.x + v1.y*v2.y;
}

inline Vector2 subVec2(Vector2 v1, Vector2 v2) {
    return { v1.x - v2.x, v1.y - v2.y };
}

inline float getDistance(Vector2 v1, Vector2 v2) {
    float dx = v1.x - v2.x;
    float dy = v1.y - v2.y;

    return sqrtf(dx*dx + dy*dy);
}

inline std::default_random_engine initEnemyGenerator() {
    return std::default_random_engine{ std::random_device{}() };
}

inline Vector2 rollRandomEnemySpawnPosition(Vector2 current_player_world_pos, std::default_random_engine& generator) {
    float spawn_pos_offset = 100;

    std::uniform_int_distribution<int> sideDist(1,4);
    int side = sideDist(generator);

    std::uniform_int_distribution<int> heightDist(-(GetScreenHeight() / 2), (GetScreenHeight() / 2));
    int randomHeight = heightDist(generator);

    std::uniform_int_distribution<int> widthDist(-(GetScreenWidth() / 2), (GetScreenWidth() / 2));
    int randomWidth = widthDist(generator);

    float world_spawn_x;
    float world_spawn_y;

    std::cout << "Enemy spawn at side: " << side << "\n";

    if (side == 1) {
        world_spawn_x = current_player_world_pos.x - ((float)GetScreenWidth() / 2) - spawn_pos_offset;
        world_spawn_y = randomHeight + current_player_world_pos.y;
    }
    else if (side == 2) {
        world_spawn_x = randomWidth + current_player_world_pos.x;
        world_spawn_y = current_player_world_pos.y - ((float)GetScreenHeight() / 2) - spawn_pos_offset;
    }
    else if (side == 3) {
        world_spawn_x = current_player_world_pos.x + ((float)GetScreenWidth() / 2) + spawn_pos_offset;
        world_spawn_y = randomHeight + current_player_world_pos.y;
    }
    else if (side == 4) {
        world_spawn_x = randomWidth + current_player_world_pos.x;
        world_spawn_y = current_player_world_pos.y + ((float)GetScreenHeight() / 2) + spawn_pos_offset;
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
