#include "game_math.h"
#include "math.h"
#include "game.h"

double Lerp(double a, double b, double t)
{
    return a + (b - a) * t;
}
float Lerpf(float a, float b, float t)
{
    return a + (b - a) * t;
}

Vector2 Vector2Lerp(Vector2 a, Vector2 b, double t)
{
    return (Vector2){Lerp(a.x, b.x, t), Lerp(a.y, b.y, t)};
}

Vector2 Vector2Add(Vector2 a, Vector2 b)
{
    return (Vector2){a.x + b.x, a.y + b.y};
}
Vector2 Vector2Sub(Vector2 a, Vector2 b)
{
    return (Vector2){a.x - b.x, a.y - b.y};
}
Vector2 Vector2Mul(Vector2 a, float scalar)
{
    return (Vector2){a.x * scalar, a.y * scalar};
}
Vector2 Vector2EisenMul(Vector2 a, Vector2 b)
{
    return (Vector2){a.x * b.x, a.y * b.y};
}
Vector2 Vector2Div(Vector2 a, float scalar)
{
    return (Vector2){a.x / scalar, a.y / scalar};
}
float Vector2Dot(Vector2 a, Vector2 b)
{
    return a.x * b.x + a.y * b.y;
}
Vector2 Vector2Perpendicular(Vector2 a)
{
    return (Vector2){-a.y, a.x};
}
float Vector2Length(Vector2 a)
{
    return sqrtf(a.x * a.x + a.y * a.y);
}
float Vector2Distance(Vector2 a, Vector2 b)
{
    return Vector2Length(Vector2Sub(a, b));
}
Vector2 Vector2Normalize(Vector2 a)
{
    if (Vector2Length(a) == 0)
        return (Vector2){0, 0};

    return Vector2Div(a, Vector2Length(a));
}
//Pra colisão
Vector2 Vector2Reflect(Vector2 v, Vector2 normal) {
    float dotProduct = v.x * normal.x + v.y * normal.y;

    Vector2 reflected;
    reflected.x = v.x - 2.0f * dotProduct * normal.x;
    reflected.y = v.y - 2.0f * dotProduct * normal.y;

    return reflected;
}

Vector2 Rect2DCenter(Rect2D r)
{
    return (Vector2){r.x + r.w / 2, r.y + r.h / 2};
}


//pedro: Todo código daqui pra baixo foi adaptado(roubad0) do meu Geometry.cs do meu projeto de RPG

bool LineLineIntersection(Line2D l1, Line2D l2, Vector2* intersectionPoint)
{
    Vector2 dir1 = Vector2Sub(l1.end, l1.start);
    Vector2 dir2 = Vector2Sub(l2.end, l2.start);

    float determinant = dir1.x * dir2.y - dir1.y * dir2.x;
    if (fabs(determinant) < 0.0001f) // Linhas são paralelas
    {
        return false;
    }

    // Calcular interseção
    Vector2 p = Vector2Sub(l2.start, l1.start);
    float t = (p.x * dir2.y - p.y * dir2.x) / determinant;
    float u = (p.x * dir1.y - p.y * dir1.x) / determinant;

    // Checar se a interseção ta dentro dos segmentos
    if (t >= 0 && t <= 1 && u >= 0 && u <= 1)
    {
        if (intersectionPoint != NULL)
            *intersectionPoint = Vector2Add(l1.start, Vector2Mul(dir1, t));
        return true;
    }
    return false;
}

bool LinePointIntersection(Line2D l, Vector2 p, Vector2* intersectionPoint)
{
    Vector2 dir = Vector2Sub(l.end, l.start);
    Vector2 pDir = Vector2Sub(p, l.start);

    float t = Vector2Dot(pDir, dir) / Vector2Dot(dir, dir);

    if (t >= 0 && t <= 1)
    {
        if (intersectionPoint != NULL)
            *intersectionPoint = Vector2Add(l.start, Vector2Mul(dir, t));
        return true;
    }
    return false;
}

bool LineCircleIntersection(Line2D l, Circle c, Vector2* intersectionPoint)
{
    Vector2 dir = Vector2Sub(l.end, l.start);
    Vector2 pDir = Vector2Sub(c.position, l.start);

    float t = Vector2Dot(pDir, dir) / Vector2Dot(dir, dir);

    if (t < 0)
        t = 0;
    else if (t > 1)
        t = 1;

    Vector2 closestPoint = Vector2Add(l.start, Vector2Mul(dir, t));
    Vector2 closestPointDir = Vector2Sub(c.position, closestPoint);

    if (Vector2Length(closestPointDir) <= c.radius)
    {
        if (intersectionPoint != NULL)
            *intersectionPoint = closestPoint;
        return true;
    }
    return false;
}

//pedro: Menos esse, esse não foi adaptado, eu tirei de https://stackoverflow.com/questions/1560492/how-to-tell-whether-a-point-is-to-the-right-or-left-side-of-a-line
bool PointLeftOfLine(Line2D l, Vector2 p)
{
    Vector2 a = l.start;
    Vector2 b = l.end;
    Vector2 c = p;
    return (b.x - a.x)*(c.y - a.y) - (b.y - a.y)*(c.x - a.x) < 0;
}

bool AABBLineIntersection(AABB aabb, Line2D l, Vector2* intersectionPoint)
{
    // Isso é burro, mas é mais fácil de fazer e eu to usando C, então vai ser rápido de qlq jeito
    // 09/08 01:00 pedro: eu to a tipo 2 horas tentando fazer isso funcionar, e agora que eu lembrei que eu tive a brilhante ideia de inverter o eixo y porquê
    // "urr durr o y é pra cima então eu vou inverter a camera hahaha", mas minha memoria muscular de programador de jogos é mais forte e 
    // fez eu digitar aabb.y + aabb.h, quando na verdade agora é aabb.y - aabb.h.
    Line2D lines[4] = {
        {aabb.x, aabb.y, aabb.x + aabb.w, aabb.y},
        {aabb.x, aabb.y, aabb.x, aabb.y - aabb.h},
        {aabb.x + aabb.w, aabb.y, aabb.x + aabb.w, aabb.y - aabb.h},
        {aabb.x, aabb.y - aabb.h, aabb.x + aabb.w, aabb.y - aabb.h}
    };

    for (int i = 0; i < 4; i++)
    {
        if (LineLineIntersection(lines[i], l, intersectionPoint))
            return true;
    }
    return false;
}

bool AABBPointIntersection(AABB aabb, Vector2 p)
{
    return p.x >= aabb.x && p.x <= aabb.x + aabb.w && p.y >= aabb.y && p.y <= aabb.y + aabb.h;
}

bool AABBAABBIntersection(AABB a, AABB b)
{
    return a.x < b.x + b.w && a.x + a.w > b.x && a.y < b.y + b.h && a.y + a.h > b.y;
}