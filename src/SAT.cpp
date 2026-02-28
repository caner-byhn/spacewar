#include "SAT.hpp"
#include "game_utils.hpp"
#include "raylib.h"
#include <math.h>



void updateSATAxisRotation(SAT& sat, float angleRad) {
    float cosA = cos(angleRad);
    float sinA = sin(angleRad);

    sat.axisX = (Vector2){ cosA, sinA };
    sat.axisY = (Vector2){ -sinA, cosA };
}

bool checkOverlapOnAxis(const SAT& A, const SAT& B, Vector2 separatingAxis) {
    float dist = fabs(dot(subVec2(B.origin, A.origin), separatingAxis));

    float rA = A.halfSize.x * fabs(dot(separatingAxis, A.axisX)) + A.halfSize.y * fabs(dot(separatingAxis, A.axisY));
    float rB = B.halfSize.x * fabs(dot(separatingAxis, B.axisX)) + B.halfSize.y * fabs(dot(separatingAxis, B.axisY));

    return dist <= rA + rB;
}

bool SATvsPoint(const SAT &A, Vector2 point) {
    {
        float dist = fabs(dot(subVec2(point, A.origin), A.axisX));
        float extent = A.halfSize.x;
        if (dist > extent) return false;
    }

    {
        float dist = fabs(dot(subVec2(point, A.origin), A.axisY));
        float extent = A.halfSize.y;
        if (dist > extent) return false;
    }

    return true;
}

bool SATvsSAT(const SAT& A, const SAT& B) {
    if (!checkOverlapOnAxis(A, B, A.axisX)) return false;
    if (!checkOverlapOnAxis(A, B, A.axisY)) return false;
    if (!checkOverlapOnAxis(A, B, B.axisX)) return false;
    if (!checkOverlapOnAxis(A, B, B.axisY)) return false;

    return true;
}

void drawSATdebugOutline(float width, float height, Vector2 origin, float angleRad, Color& color) {
    float halfW = width / 2;
    float halfH = height / 2;

    Vector2 corners[4] = {
        { -halfW, -halfH },
        { halfW, -halfH },
        { halfW, halfH },
        { -halfW, halfH }
    };

    for (int i = 0; i < 4; i++) {
        float x = corners[i].x;
        float y = corners[i].y;

        corners[i].x = origin.x + x * cosf(angleRad) - y * sinf(angleRad);
        corners[i].y = origin.y + x * sinf(angleRad) + y * cosf(angleRad);
    }

    for (int i = 0; i < 4; i++) {
        DrawLineEx(corners[i], corners[(i + 1) % 4], 2.0f, color);
    }
}
