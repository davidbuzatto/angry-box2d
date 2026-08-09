/**
 * @file GameWorld.h
 * @author Prof. Dr. David Buzatto
 * @brief GameWorld struct and function declarations.
 * 
 * @copyright Copyright (c) 2026
 */
#pragma once

#include <stdbool.h>

#include "box2d/box2d.h"
#include "raylib/raylib.h"

#include "Entity.h"
#include "Macros.h"

typedef enum GameState {
    STATE_PLAYING,
    STATE_WON,
    STATE_LOST,
} GameState;

typedef struct GameWorld {
    
    b2WorldId worldId;
    Entity entities[MAX_ENTITIES];

    int entityCount;
    int birdEntityIndex;
    int birdsRemaining;
    Vector2 slingAnchor;

    bool dragging;
    Vector2 dragCurrent;
    GameState state;
    bool debugDraw;

} GameWorld;

/**
 * @brief Creates a dinamically allocated GameWorld struct instance.
 */
GameWorld *createGameWorld( void );

/**
 * @brief Destroys a GameWindow object and its dependecies.
 */
void destroyGameWorld( GameWorld *gw );

/**
 * @brief Reads user input and updates the state of the game.
 */
void updateGameWorld( GameWorld *gw, float delta );

/**
 * @brief Draws the state of the game.
 */
void drawGameWorld( GameWorld *gw );