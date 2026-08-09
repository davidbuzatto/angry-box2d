#pragma once

#include "box2d/box2d.h"
#include "raylib/raylib.h"

// converts a real world point (meters, Y up) to screen (pixels, Y down)
Vector2 b2ToScreen( b2Vec2 p );

// converts a screen point (pixels, Y down) to real world point (meters, Y up) 
b2Vec2 screenToB2( Vector2 p );