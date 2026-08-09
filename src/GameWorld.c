/**
 * @file GameWorld.c
 * @author Prof. Dr. David Buzatto
 * @brief GameWorld implementation.
 * 
 * @copyright Copyright (c) 2026
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "raylib/raylib.h"
//#include "raylib/raymath.h"
//#define RAYGUI_IMPLEMENTATION    // to use raygui, comment these three lines.
//#include "raylib/raygui.h"       // other compilation units must only include
//#undef RAYGUI_IMPLEMENTATION     // raygui.h

#include "box2d/box2d.h"

#include "DebugDraw.h"
#include "Entity.h"
#include "GameWorld.h"
#include "ResourceManager.h"

static void addBox( 
    GameWorld *gw, b2BodyType bodyType, float x, float y, 
    float halfWidth, float halfHeight, Color color, EntityType type );

static void addCircle( 
    GameWorld *gw, b2BodyType bodyType, float x, float y, 
    float radius, Color color, EntityType type );

/**
 * @brief Creates a dinamically allocated GameWorld struct instance.
 */
GameWorld *createGameWorld( void ) {

    GameWorld *gw = (GameWorld*) malloc( sizeof( GameWorld ) );

    b2WorldDef worldDef = b2DefaultWorldDef();
    gw->worldId = b2CreateWorld( &worldDef );

    gw->entityCount = 0;

    float screenWidthMeters = GetScreenWidth() / PIXELS_PER_METER;
    const float groundHeightMeters = 1.0f;

    // ground
    addBox( 
        gw, b2_staticBody, 
        screenWidthMeters / 2.0f, groundHeightMeters / 2.0f, 
        screenWidthMeters / 2.0f, groundHeightMeters / 2.0f,
        DARKGREEN, ENTITY_GROUND
    );

    // block pyramid
    const float blockSize = 1.0f;
    const int pyramidRows = 4;
    const float pyramidCenterX = screenWidthMeters * 0.65f;

    for ( int row = 0; row < pyramidRows; row++ ) {

        int blocksInRow = pyramidRows - row;
        float rowWidth = blocksInRow * blockSize;
        float startX = pyramidCenterX - rowWidth / 2.0f + blockSize / 2.0f;
        float y = groundHeightMeters + row * blockSize + blockSize / 2.0f;

        for ( int i = 0; i < blocksInRow; i++ ) {
            float x = startX + i * blockSize;
            addBox( gw, b2_dynamicBody, x, y, blockSize / 2.0f, blockSize / 2.0f, BROWN, ENTITY_BLOCK );
        }

    }

    // pigs
    const float pigRadius = 0.4f;
    addCircle( 
        gw, b2_dynamicBody, pyramidCenterX - blockSize * 2.5f, 
        groundHeightMeters + pigRadius, pigRadius, 
        GREEN, ENTITY_PIG
    );
    addCircle( 
        gw, b2_dynamicBody, pyramidCenterX + blockSize * 2.5f,
        groundHeightMeters + pigRadius, pigRadius, 
        GREEN, ENTITY_PIG
    );
    addCircle( 
        gw, b2_dynamicBody, pyramidCenterX,
        groundHeightMeters + pyramidRows * blockSize + pigRadius, pigRadius,
        GREEN, ENTITY_PIG
    );
    
    gw->birdEntityIndex = -1;
    gw->birdsRemaining = 3;
    gw->slingAnchor = (Vector2) { 0 };
    gw->dragging = false;
    gw->dragCurrent = (Vector2) { 0 };
    gw->state = STATE_PLAYING;
    gw->debugDraw = true;

    return gw;

}

/**
 * @brief Destroys a GameWindow object and its dependecies.
 */
void destroyGameWorld( GameWorld *gw ) {
    b2DestroyWorld( gw->worldId );
    free( gw );
}

/**
 * @brief Reads user input and updates the state of the game.
 */
void updateGameWorld( GameWorld *gw, float delta ) {

    if ( IsKeyPressed( KEY_F1 ) ) {
        gw->debugDraw = !gw->debugDraw;
    }

    b2World_Step( gw->worldId, delta, 4 );

}

/**
 * @brief Draws the state of the game.
 */
void drawGameWorld( GameWorld *gw ) {

    BeginDrawing();
    ClearBackground( WHITE );

    for ( int i = 0; i < gw->entityCount; i++ ) {
        drawEntity( &gw->entities[i] );
    }

    if ( gw->debugDraw ) {
        b2DebugDraw debugDraw = createGameDebugDraw();
        b2World_Draw( gw->worldId, &debugDraw );
    }

    EndDrawing();

}

static void addBox( 
    GameWorld *gw, b2BodyType bodyType, float x, float y, 
    float halfWidth, float halfHeight, Color color, EntityType type ) {

    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = bodyType;
    bodyDef.position = (b2Vec2) { x, y };
    b2BodyId bodyId = b2CreateBody( gw->worldId, &bodyDef );

    b2Polygon box = b2MakeBox( halfWidth, halfHeight );
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    b2CreatePolygonShape( bodyId, &shapeDef, &box );

    gw->entities[gw->entityCount++] = (Entity) {
        .bodyId = bodyId,
        .type = type,
        .kind = SHAPE_BOX,
        .size = { halfWidth * 2.0f, halfHeight * 2.0f },
        .radius = 0.0f,
        .color = color,
        .alive = true
    };

}

static void addCircle( 
    GameWorld *gw, b2BodyType bodyType, float x, float y, 
    float radius, Color color, EntityType type ) {

    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = bodyType;
    bodyDef.position = (b2Vec2) { x, y };
    b2BodyId bodyId = b2CreateBody( gw->worldId, &bodyDef );

    b2Circle circle = { .center = { 0 }, .radius = radius };
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = 1.0f;
    shapeDef.material.friction = 0.3f;
    shapeDef.material.restitution = 0.3f;
    b2CreateCircleShape( bodyId, &shapeDef, &circle );

    gw->entities[gw->entityCount++] = (Entity) {
        .bodyId = bodyId,
        .type = type,
        .kind = SHAPE_CIRCLE,
        .size = { 0 },
        .radius = radius,
        .color = color,
        .alive = true
    };

}
