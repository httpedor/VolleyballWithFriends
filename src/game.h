#if !defined(GAME_H)
#define GAME_H
#include "game_math.h"
#include "camera.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include "animation.h"
#include "player.h"

#define TICK_RATE 60
#define MAX_PLAYER_COUNT 4


typedef struct {
    Animator* animator;
    SDL_Texture* projection_texture;

    Vector2 position;
    float z;
    Vector2 velocity;
    float zVelocity;
    float radius;
} Ball;

typedef struct {
    bool KEYS[SDL_NUM_SCANCODES];
    bool shouldQuit;
    Vector2 screenSize;
    Camera camera;
    double timeScale;

    bool paused;
    uint64_t deltaTime;
    Player players[4];
    Ball ball;
    SDL_Texture* background;
    SDL_Texture* netTexture;
    Line2D net;

    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* font;
} GameData;


GameData* GameInit();
void GameQuit();
Player* GameGetPlayer(int index);
Ball* GameGetBall();
Camera* GameGetCamera();
GameData* GameGetData();
Vector2 GameGetScreenSize();
Rect2D GameGetCameraRect();
bool GameIsKeyDown(SDL_Scancode scancode);
double GameGetTimeScale();

Vector2 GetMousePosScreen();
Vector2 GetMousePosWorld();

void GameInput();
void GameProcess(double dt);
void GameRender(double dt, double fps);

#endif // GAME_H
