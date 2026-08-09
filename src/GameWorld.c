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
#include "raylib/raymath.h"
//#define RAYGUI_IMPLEMENTATION    // to use raygui, comment these three lines.
//#include "raylib/raygui.h"       // other compilation units must only include
//#undef RAYGUI_IMPLEMENTATION     // raygui.h

#include "box2d/box2d.h"

#include "DebugDraw.h"
#include "Entity.h"
#include "GameWorld.h"
#include "ResourceManager.h"
#include "Utils.h"

static void buildLevel( GameWorld *gw );

static void addBox( 
    GameWorld *gw, b2BodyType bodyType, float x, float y, 
    float halfWidth, float halfHeight, float density, Color color, EntityType type );

static void addCircle( 
    GameWorld *gw, b2BodyType bodyType, float x, float y, 
    float radius, float density, Color color, EntityType type );

static void spawnBird( GameWorld *gw );
static void resetLevel( GameWorld *gw );

/**
 * @brief Creates a dinamically allocated GameWorld struct instance.
 */
GameWorld *createGameWorld( void ) {

    GameWorld *gw = (GameWorld*) malloc( sizeof( GameWorld ) );

    b2WorldDef worldDef = b2DefaultWorldDef();
    gw->worldId = b2CreateWorld( &worldDef );

    gw->dragging = false;
    gw->dragCurrent = (Vector2) { 0 };
    gw->state = STATE_PLAYING;
    gw->debugDraw = true;

    buildLevel( gw );

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

    if ( gw->state != STATE_PLAYING ) {
        if ( IsKeyPressed( KEY_R ) ) {
            resetLevel( gw );
        }
        return;
    }

    if ( gw->birdEntityIndex != -1 ) {

        Entity *bird = &gw->entities[gw->birdEntityIndex];
        Vector2 mouse = GetMousePosition();

        if ( !gw->dragging && IsMouseButtonPressed( MOUSE_BUTTON_LEFT ) ) {
            Vector2 birdScreenPos = b2ToScreen( b2Body_GetPosition( bird->bodyId ) );
            if ( CheckCollisionPointCircle( mouse, birdScreenPos, bird->radius * PIXELS_PER_METER * 1.5f ) ) {
                gw->dragging = true;
            }
        }

        if ( gw->dragging ) {

            Vector2 offset = Vector2Subtract( mouse, gw->slingAnchor );
            float dist = Vector2Length( offset );
            float maxDragPixels = MAX_DRAG_METERS * PIXELS_PER_METER;
            
            if ( dist > maxDragPixels ) {
                offset = Vector2Scale( offset, maxDragPixels / dist );
            }

            gw->dragCurrent = Vector2Add( gw->slingAnchor, offset );
            b2Vec2 birdPos = screenToB2( gw->dragCurrent );
            b2Body_SetTransform( bird->bodyId, birdPos, b2Rot_identity );

            // launching
            if ( IsMouseButtonReleased( MOUSE_BUTTON_LEFT ) ) {

                b2Vec2 anchorMeters = screenToB2( gw->slingAnchor );
                b2Vec2 launchDir = { anchorMeters.x - birdPos.x, anchorMeters.y - birdPos.y };
                
                b2Body_SetType( bird->bodyId, b2_dynamicBody );
                b2Body_SetLinearVelocity( 
                    bird->bodyId,
                    (b2Vec2) {
                        launchDir.x * LAUNCH_STRENGTH,
                        launchDir.y * LAUNCH_STRENGTH
                    }
                );

                gw->dragging = false;

            }

        }

    }

    b2World_Step( gw->worldId, delta, 4 );

    // detects pigs that were hit with enougth strength
    b2ContactEvents events = b2World_GetContactEvents( gw->worldId );
    for ( int i = 0; i < events.hitCount; i++ ) {

        b2ContactHitEvent *hit = &events.hitEvents[i];
        if ( hit->approachSpeed < HIT_SPEED_THRESHOLD ) {
            continue;
        }

        b2BodyId bodyA = b2Shape_GetBody( hit->shapeIdA );
        b2BodyId bodyB = b2Shape_GetBody( hit->shapeIdB );

        for ( int j = 0; j < gw->entityCount; j++ ) {
            Entity *e = &gw->entities[j];
            if ( e->type == ENTITY_PIG && e->alive && 
            ( B2_ID_EQUALS( e->bodyId, bodyA ) || B2_ID_EQUALS( e->bodyId, bodyB ) ) ) {
                e->alive = false;
                b2DestroyBody( e->bodyId );
            }
        }

    }

    // victory
    bool anyPigAlive = false;
    for ( int i = 0; i < gw->entityCount; i++ ) {
        if ( gw->entities[i].type == ENTITY_PIG && gw->entities[i].alive ) {
            anyPigAlive = true;
            break;
        }
    }
    if ( !anyPigAlive ) {
        gw->state = STATE_WON;
        return;
    }

    // current bird stopped or is out of screen bounds?
    if ( gw->birdEntityIndex != -1 && !gw->dragging ) {

        Entity *bird = &gw->entities[gw->birdEntityIndex];

        if ( b2Body_GetType( bird->bodyId ) == b2_dynamicBody ) {

            float speed = b2Length( b2Body_GetLinearVelocity( bird->bodyId ) );
            Vector2 screenPos = b2ToScreen( b2Body_GetPosition( bird->bodyId ) );
            bool offScreen = screenPos.x < -100 || screenPos.x > GetScreenWidth() + 100 || screenPos.y > GetScreenHeight() + 100;

            if ( speed < SETTLE_SPEED_THRESHOLD || offScreen ) {

                b2DestroyBody( bird->bodyId );
                bird->alive = false;
                gw->birdEntityIndex = -1;

                if ( gw->birdsRemaining > 0 ) {
                    gw->birdsRemaining--;
                    spawnBird( gw );
                } else {
                    gw->state = STATE_LOST;
                }

            }

        }

    }

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

    if ( gw->dragging ) {
        DrawLineEx( gw->slingAnchor, gw->dragCurrent, 4.0f, DARKBROWN );
    }

    if ( gw->debugDraw ) {
        b2DebugDraw debugDraw = createGameDebugDraw();
        b2World_Draw( gw->worldId, &debugDraw );
    }

    // hud
    DrawFPS( 20, 20 );
    DrawText( TextFormat( "Birds remaining: %d", gw->birdsRemaining ), 20, 50, 20, BLACK );

    if ( gw->state != STATE_PLAYING ) {

        const char *text = gw->state == STATE_WON ? 
            "You Win! Press R to restart!" :
            "You Lose (no more birds)... Press R to restart!";
        
        int textWidth = MeasureText( text, 30 );
        int x = GetScreenWidth() / 2 - textWidth / 2;
        int y = GetScreenHeight() / 2;

        DrawRectangle( x - 10, y - 20, textWidth + 20, 45, Fade( BLACK, 0.7f ) );
        DrawText( text, x, y - 10, 30, WHITE );

    }

    EndDrawing();

}

static void buildLevel( GameWorld *gw ) {

    gw->entityCount = 0;

    float screenWidthMeters = GetScreenWidth() / PIXELS_PER_METER;
    const float groundHeightMeters = 1.0f;

    // ground
    addBox( 
        gw, b2_staticBody, 
        screenWidthMeters / 2.0f, groundHeightMeters / 2.0f, 
        screenWidthMeters / 2.0f, groundHeightMeters / 2.0f,
        1.0f, DARKGREEN, ENTITY_GROUND
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
            addBox( gw, b2_dynamicBody, x, y, blockSize / 2.0f, blockSize / 2.0f, 1.0f, BROWN, ENTITY_BLOCK );
        }

    }

    // pigs
    const float pigRadius = 0.4f;
    addCircle( 
        gw, b2_dynamicBody, pyramidCenterX - blockSize * 2.5f, 
        groundHeightMeters + pigRadius, pigRadius, 
        1.0f, GREEN, ENTITY_PIG
    );
    addCircle( 
        gw, b2_dynamicBody, pyramidCenterX + blockSize * 2.5f,
        groundHeightMeters + pigRadius, pigRadius, 
        1.0f, GREEN, ENTITY_PIG
    );
    addCircle( 
        gw, b2_dynamicBody, pyramidCenterX,
        groundHeightMeters + pyramidRows * blockSize + pigRadius, pigRadius,
        1.0f, GREEN, ENTITY_PIG
    );
    
    // sling + bird
    float slingX = screenWidthMeters * 0.15f;
    float slingY = groundHeightMeters + 2.0f;
    gw->slingAnchor = b2ToScreen( (b2Vec2) { slingX, slingY } );

    const float birdRadius = BIRD_RADIUS;
    addCircle( 
        gw, b2_kinematicBody, slingX, slingY, birdRadius,
        BIRD_DENSITY, RED, ENTITY_BIRD
    );
    gw->birdEntityIndex = gw->entityCount - 1;
    gw->birdsRemaining = TOTAL_BIRDS - 1;

}

static void addBox( 
    GameWorld *gw, b2BodyType bodyType, float x, float y, 
    float halfWidth, float halfHeight, float density, Color color, EntityType type ) {

    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = bodyType;
    bodyDef.position = (b2Vec2) { x, y };
    b2BodyId bodyId = b2CreateBody( gw->worldId, &bodyDef );

    b2Polygon box = b2MakeBox( halfWidth, halfHeight );
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.enableHitEvents = true;
    shapeDef.density = density;
    shapeDef.material.friction = 0.35f;
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
    float radius, float density, Color color, EntityType type ) {

    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = bodyType;
    bodyDef.position = (b2Vec2) { x, y };
    b2BodyId bodyId = b2CreateBody( gw->worldId, &bodyDef );

    b2Circle circle = { .center = { 0 }, .radius = radius };
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.enableHitEvents = true;
    shapeDef.density = density;
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

static void spawnBird( GameWorld *gw ) {

    b2Vec2 anchorMeters = screenToB2( gw->slingAnchor );

    addCircle( 
        gw, b2_kinematicBody, anchorMeters.x, anchorMeters.y, BIRD_RADIUS,
        BIRD_DENSITY, RED, ENTITY_BIRD
    );

    gw->birdEntityIndex = gw->entityCount - 1;

}

static void resetLevel( GameWorld *gw ) {

    b2DestroyWorld( gw->worldId );

    b2WorldDef worldDef = b2DefaultWorldDef();
    gw->worldId = b2CreateWorld( &worldDef );

    gw->dragging = false;
    gw->dragCurrent = (Vector2) { 0 };
    gw->state = STATE_PLAYING;

    buildLevel( gw );

}
