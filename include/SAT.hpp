#pragma once

#include "raylib.h"

struct SAT {
    Vector2 axisX;
    Vector2 axisY;
    Vector2 origin;
    Vector2 halfSize;
};


void updateSATAxisRotation(SAT& sat, float angleRad);
bool checkOverlapOnAxis(const SAT& A, const SAT& B, Vector2 separatingAxis);
bool SATvsPoint(const SAT& A, Vector2 point);
bool SATvsSAT(const SAT& A, const SAT& B);

void drawSATdebugOutline(float width, float height, Vector2 origin, float angleRad, Color& color);
