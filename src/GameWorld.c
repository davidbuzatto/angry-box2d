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

#include "Entity.h"
#include "GameWorld.h"
#include "ResourceManager.h"

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

    // static ground
    {
        b2BodyDef groundBodyDef = b2DefaultBodyDef();
        groundBodyDef.type = b2_staticBody;
        groundBodyDef.position = (b2Vec2) { 
            screenWidthMeters / 2.0f, // half width
            groundHeightMeters / 2.0f // half height
        };
        b2BodyId groundBodyId = b2CreateBody( gw->worldId, &groundBodyDef );
        b2Polygon groundBox = b2MakeBox( 
            screenWidthMeters / 2.0f, // half width
            groundHeightMeters / 2.0f // half height
        );
        b2ShapeDef groundShapeDef = b2DefaultShapeDef();
        b2CreatePolygonShape( groundBodyId, &groundShapeDef, &groundBox );

        gw->entities[gw->entityCount++] = (Entity) {
            .bodyId = groundBodyId,
            .type = ENTITY_GROUND,
            .kind = SHAPE_BOX,
            .size = { screenWidthMeters, groundHeightMeters },
            .radius = 0.0f,
            .color = DARKGREEN,
            .alive = true
        };
    }

    // test box (dynamic)
    {
        b2BodyDef boxBodyDef = b2DefaultBodyDef();
        boxBodyDef.type = b2_dynamicBody;
        boxBodyDef.position = (b2Vec2) { screenWidthMeters / 2.0f, 6.0f };
        b2BodyId boxBodyId = b2CreateBody( gw->worldId, &boxBodyDef );

        b2Polygon box = b2MakeBox( 0.5f, 0.5f );
        b2ShapeDef boxShapeDef = b2DefaultShapeDef();
        boxShapeDef.density = 1.0f;
        boxShapeDef.material.friction = 0.3f;
        boxShapeDef.material.restitution = 0.3f;
        b2CreatePolygonShape( boxBodyId, &boxShapeDef, &box );

        gw->entities[gw->entityCount++] = (Entity) {
            .bodyId = boxBodyId,
            .type = ENTITY_BLOCK,
            .kind = SHAPE_BOX,
            .size = { 1.0f, 1.0f },
            .radius = 0.0f,
            .color = BLUE,
            .alive = true
        };
    }
    
    gw->birdEntityIndex = -1;
    gw->birdsRemaining = 10;
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

    EndDrawing();

}
