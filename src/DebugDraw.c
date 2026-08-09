#include "box2d/box2d.h"
#include "raylib/raylib.h"

#include "DebugDraw.h"
#include "Macros.h"
#include "Utils.h"

static Color hexToColor( b2HexColor hex ) {
    return (Color) {
        (unsigned char) ( ( hex >> 16 ) & 0xFF ),
        (unsigned char) ( ( hex >>  8 ) & 0xFF ),
        (unsigned char) (           hex & 0xFF ),
        255
    };
}

static void drawSolidPolygon( 
    b2Transform transform, 
    const b2Vec2 *vertices, 
    int vertexCount, 
    float radius, 
    b2HexColor color, 
    void *context ) {


    (void) radius;
    (void) context;

    Vector2 points[B2_MAX_POLYGON_VERTICES];
    for ( int i = 0; i < vertexCount; i++ ) {
        b2Vec2 worldPoint = b2TransformPoint( transform, vertices[i] );
        points[i] = b2ToScreen( worldPoint );
    }

    Color c = hexToColor( color );
    DrawTriangleFan( points, vertexCount, Fade( c, 0.5f ) );

    for ( int i = 0; i < vertexCount; i++ ) {
        Vector2 a = points[i];
        Vector2 b = points[( i + 1 ) % vertexCount];
        DrawLineV( a, b, c );
    }

}

static void drawSolidCircle(
    b2Transform transform, 
    float radius, 
    b2HexColor color, 
    void *context ) {

    (void) context;
    Vector2 center = b2ToScreen( transform.p );
    float screenRadius = radius * PIXELS_PER_METER;
    Color c = hexToColor( color );

    DrawCircleV( center, screenRadius, Fade( c, 0.5f ) );
    DrawCircleLinesV( center, screenRadius, c );

}

static void drawSegment( b2Vec2 p1, b2Vec2 p2, b2HexColor color, void *context ) {
    (void) context;
    DrawLineV( b2ToScreen( p1 ), b2ToScreen( p2 ), hexToColor( color ) );
}

b2DebugDraw createGameDebugDraw( void ) {

    b2DebugDraw debugDraw = b2DefaultDebugDraw();

    debugDraw.DrawSolidPolygonFcn = drawSolidPolygon;
    debugDraw.DrawSolidCircleFcn = drawSolidCircle;
    debugDraw.DrawSegmentFcn = drawSegment;

    debugDraw.drawShapes = true;

    return debugDraw;

}