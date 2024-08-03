#if !defined(GAME_MATH_H)
#define GAME_MATH_H
#include <SDL_rect.h>

#define Vector2 SDL_FPoint
#define Rect2D SDL_FRect
#define AABB Rect2D
#define true SDL_TRUE
#define false SDL_FALSE
#define bool uint8_t 
#define PIXELS_PER_METER 25


typedef struct
{
    Vector2 start;
    Vector2 end;
} Line2D;

typedef struct
{
    Vector2 position;
    float radius;
} Circle;
double Lerp(double a, double b, double t);
float Lerpf(float a, float b, float t);

Vector2 Vector2Add(Vector2 a, Vector2 b);
Vector2 Vector2Sub(Vector2 a, Vector2 b);
Vector2 Vector2Mul(Vector2 a, float scalar);
Vector2 Vector2EisenMul(Vector2 a, Vector2 b);
Vector2 Vector2Div(Vector2 a, float scalar);
float Vector2Dot(Vector2 a, Vector2 b);
Vector2 Vector2Perpendicular(Vector2 a);
float Vector2Length(Vector2 a);
float Vector2Distance(Vector2 a, Vector2 b);
Vector2 Vector2Normalize(Vector2 a);
Vector2 Vector2Lerp(Vector2 a, Vector2 b, double t);

Vector2 Rect2DCenter(Rect2D r);

bool LineLineIntersection(Line2D l1, Line2D l2, Vector2* intersectionPoint);
bool LinePointIntersection(Line2D l, Vector2 p, Vector2* intersectionPoint);
bool LineCircleIntersection(Line2D l, Circle c, Vector2* intersectionPoint);
bool AABBLineIntersection(AABB aabb, Line2D l, Vector2* intersectionPoint);
bool AABBPointIntersection(AABB aabb, Vector2 p);
bool AABBAABBIntersection(AABB a, AABB b);

#endif // GAME_MATH_H
