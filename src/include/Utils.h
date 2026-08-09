#pragma once

#include "box2d/box2d.h"
#include "raylib/raylib.h"

// converts a real world point (meters, Y up) to screen (pixels, Y down)
Vector2 b2ToScreen( b2Vec2 p );