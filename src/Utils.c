#include "box2d/box2d.h"
#include "raylib/raylib.h"

#include "Utils.h"
#include "Macros.h"

Vector2 b2ToScreen( b2Vec2 p ) {
    return (Vector2) {
        p.x * PIXELS_PER_METER,
        GetScreenHeight() - p.y * PIXELS_PER_METER
    };
}

b2Vec2 screenToB2( Vector2 p ) {
    return (b2Vec2) {
        p.x / PIXELS_PER_METER,
        ( GetScreenHeight() - p.y ) / PIXELS_PER_METER
    };
}