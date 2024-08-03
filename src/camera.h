#ifndef CAMERA_H
#define CAMERA_H
#include "game_math.h"
#include "animation.h"
#include <SDL.h>

typedef struct {
    Vector2 position;
    float zoom;
} Camera;

Rect2D ApplyCameraTransformR(Camera* camera, Rect2D r);
Vector2 ApplyCameraTransformV(Camera* camera, Vector2 vec);
Vector2 ScreenToWorld(Vector2 vec);
Vector2 WorldToScreen(Vector2 vec);

Rect2D ScreenToWorldRect(Rect2D rect);
Rect2D WorldToScreenRect(Rect2D rect);


void CameraRenderRect(Camera* camera, Rect2D rect, SDL_Renderer* renderer);
void CameraRenderRectF(Camera* camera, Rect2D rect, SDL_Renderer* renderer);
void CameraRenderRectOutlineF(Camera* camera, Rect2D rect, SDL_Renderer* renderer);
void CameraRenderCircle(Camera* camera, Vector2 pos, float radius, SDL_Renderer* renderer);
void CameraRenderCircleOutline(Camera* camera, Vector2 pos, float radius, SDL_Renderer* renderer);
void CameraRenderAnimator(Camera* camera, Animator* animator, Vector2 position, Vector2 size);
void CameraRenderTexture(Camera* camera, SDL_Texture* texture, Vector2 position, Vector2 size);

#endif