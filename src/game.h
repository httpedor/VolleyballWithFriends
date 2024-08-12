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
#define BALL_TRAIL_LENGTH 150

#define Y_MAX 100
#define Y_MIN -160
#define X_MAX 300
#define X_MIN -330

typedef enum {
    PATH_PARABOLIC = 0,
    PATH_LINEAR = 1,
    PATH_LOGARITHMIC = 2,
} PathType;

typedef enum {
    TEAM_LEFT = 0,
    TEAM_RIGHT = 1,
} Team;

typedef struct {
    double deadzoneLeft;
    double deadzoneRight;
} Config;

typedef struct {
    Animator* animator;
    SDL_Texture* projection_texture;

    Vector2 position;
    float z;
    float radius;

    float pathProgress;
    Vector2 targetPosition;
    Vector2 startPosition;
    float pathDuration;
    float pathHeight;
    float logShapeConstant;
    PathType pathType;

    bool isStarSet;
    bool isStarSpike;
    bool isServed;

    uint32_t trailColor;
    Vector2 trail[BALL_TRAIL_LENGTH];
    float trailZ[BALL_TRAIL_LENGTH];
} Ball;

typedef struct {
    uint32_t KEYS[SDL_NUM_SCANCODES];
    bool shouldQuit;
    Vector2 screenSize;
    Camera camera;
    double timeScale;

    int lastPlayerToTouchBall;

    bool paused;
    uint64_t deltaTime;
    Player players[4];
    Ball ball;
    SDL_Texture* background;
    SDL_Texture* netTexture;
    Line2D net;
    int teamTouchCount[2];
    Team lastTeamToServe;
    int teamScore[2];
    int teamLastServer[2];

    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* font;
} GameData;


GameData* GameInit();
void GameQuit();
Player* GameGetPlayer(int index);
Player* GameGetOtherPlayerInTeam(int index);
Ball* GameGetBall();
Camera* GameGetCamera();
GameData* GameGetData();
Config* GameGetConfig();
Vector2 GameGetScreenSize();
Rect2D GameGetCameraRect();
bool GameIsKeyDown(SDL_Scancode scancode);
uint32_t GameMillisKeyPressed(SDL_Scancode scancode);
double GameGetTimeScale();
float GameRandom(float min, float max);

Vector2 GetMousePosScreen();
Vector2 GetMousePosWorld();

Team GameOtherTeam(Team team);
void GameTouchBall(int player, Vector2 target, float duration, float height, PathType pathType, bool countTouch, uint32_t color);
void GameSetupRally(Team server);

void GameInput(double dt);
void GameProcess(double dt);
void GameRender(double dt, double fps);

#endif // GAME_H
