#include "camera.h"
#include "game.h"

SDL_FRect ApplyCameraTransformR(Camera* camera, SDL_FRect r)
{
    r.x -= camera->position.x;
    r.y -= camera->position.y;
    r.x *= camera->zoom;
    r.y *= camera->zoom;
    r.w *= camera->zoom;
    r.h *= camera->zoom;
    return r;
}

Vector2 ApplyCameraTransformV(Camera* camera, Vector2 vec)
{
    vec.x -= camera->position.x;
    vec.y -= camera->position.y;
    vec = Vector2Mul(vec, camera->zoom);
    return vec;
}

Vector2 ScreenToWorld(Vector2 vec)
{
    SDL_FPoint screenSize = GameGetScreenSize();
    return (Vector2){vec.x - (screenSize.x/2), -vec.y + (screenSize.y/2)};
}

Vector2 WorldToScreen(Vector2 vec)
{
    SDL_FPoint screenSize = GameGetScreenSize();
    return (Vector2){vec.x + (screenSize.x/2), -vec.y + (screenSize.y/2)};
}

Rect2D ScreenToWorldRect(Rect2D rect)
{
    Vector2 pos = ScreenToWorld((Vector2){rect.x, rect.y});
    Vector2 size = (Vector2){rect.w, rect.h};
    return (Rect2D){pos.x, pos.y, size.x, size.y};
}

Rect2D WorldToScreenRect(Rect2D rect)
{
    Vector2 pos = WorldToScreen((Vector2){rect.x, rect.y});
    Vector2 size = (Vector2){rect.w, rect.h};
    return (Rect2D){pos.x, pos.y, size.x, size.y};
}

void CameraRenderRect(Camera* camera, Rect2D rect, SDL_Renderer* renderer)
{
    Rect2D r = ApplyCameraTransformR(camera, WorldToScreenRect(rect));

    SDL_RenderFillRectF(renderer, &r);
}
void CameraRenderRectF(Camera* camera, Rect2D rect, SDL_Renderer* renderer)
{
    SDL_FRect r = ApplyCameraTransformR(camera, WorldToScreenRect(rect));
    SDL_RenderFillRectF(renderer, &r);
}

void CameraRenderRectOutlineF(Camera* camera, Rect2D rect, SDL_Renderer* renderer)
{
    SDL_FRect r = ApplyCameraTransformR(camera, WorldToScreenRect(rect));
    SDL_RenderDrawRectF(renderer, &r);
}

void pixel(SDL_Renderer* renderer, int x, int y)
{
    SDL_RenderDrawPoint(renderer, x, y);
}

//Eu provavelmente vou mudar esse código pra usar uma textura de circulo
//Mas até lá, usando código adptado de: https://stackoverflow.com/questions/38334081/how-to-draw-circles-arcs-and-vector-graphics-in-sdl
void CameraRenderCircle(Camera* camera, Vector2 pos, float radius, SDL_Renderer* renderer)
{
    pos = ApplyCameraTransformV(camera, WorldToScreen(pos));

    for (int w = 0; w < radius * 2; w++)
    {
        for (int h = 0; h < radius * 2; h++)
        {
            int dx = radius - w; // horizontal offset
            int dy = radius - h; // vertical offset
            if ((dx*dx + dy*dy) <= (radius * radius))
            {
                SDL_RenderDrawPoint(renderer, pos.x + dx, pos.y + dy);
            }
        
        }
    }
}

void CameraRenderCircleOutline(Camera* camera, Vector2 pos, float radius, SDL_Renderer* renderer)
{
    pos = ApplyCameraTransformV(camera, WorldToScreen(pos));

    for (int w = 0; w < radius * 2; w++)
    {
        for (int h = 0; h < radius * 2; h++)
        {
            int dx = radius - w; // horizontal offset
            int dy = radius - h; // vertical offset
            if ((dx*dx + dy*dy) <= (radius * radius))
            {
                SDL_RenderDrawPoint(renderer, pos.x + dx, pos.y + dy);
            }
        
        }
    }
}

void CameraRenderAnimator(Camera* camera, Animator* animator, Vector2 position, Vector2 size)
{
    position = ApplyCameraTransformV(camera, WorldToScreen(position));
    size = Vector2Mul(size, camera->zoom);

    AnimatorRender(animator, position, size);
}

void CameraRenderTexture(Camera* camera, SDL_Texture* texture, Vector2 position, Vector2 size)
{
    position = ApplyCameraTransformV(camera, WorldToScreen(position));
    size = Vector2Mul(size, camera->zoom);

    SDL_Rect dst = {position.x, position.y, size.x, size.y};
    SDL_RenderCopy(GameGetData()->renderer, texture, NULL, &dst);
}