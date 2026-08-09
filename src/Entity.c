#include "box2d/box2d.h"
#include "raylib/raylib.h"

#include "Entity.h"
#include "Macros.h"
#include "Utils.h"

void drawEntity( Entity *e ) {

    if ( !e->alive ) {
        return;
    }

    b2Vec2 pos = b2Body_GetPosition( e->bodyId );
    float angleRed = b2Rot_GetAngle( b2Body_GetRotation( e->bodyId ) );
    Vector2 screenPos = b2ToScreen( pos );

    if ( e->kind == SHAPE_BOX ) {
        Rectangle rec = {
            screenPos.x,
            screenPos.y,
            e->size.x * PIXELS_PER_METER,
            e->size.y * PIXELS_PER_METER,
        };
        Vector2 origin = { rec.width / 2.0f, rec.height / 2.0f };
        DrawRectanglePro( rec, origin, -angleRed * RAD2DEG, e->color );
    } else {
        DrawCircleV( screenPos, e->radius * PIXELS_PER_METER, e->color );
    }

}