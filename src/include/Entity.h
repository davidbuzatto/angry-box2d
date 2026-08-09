#pragma once

#include <stdbool.h>

#include "box2d/box2d.h"
#include "raylib/raylib.h"

typedef enum EntityType {
    ENTITY_GROUND,
    ENTITY_BLOCK,
    ENTITY_PIG,
    ENTITY_BIRD,
} EntityType;

typedef enum ShapeKind {
    SHAPE_BOX,
    SHAPE_CIRCLE,
} ShapeKind;

typedef struct Entity {

    b2BodyId bodyId;
    EntityType type;
    ShapeKind kind;

    Vector2 size; // SHAPE_BOX
    float radius; // SHAPE_CIRCLE

    Color color;
    bool alive;

} Entity;

void drawEntity( Entity *e );