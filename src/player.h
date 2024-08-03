#ifndef PLAYER_H
#define PLAYER_H
#include <SDL.h>
#include "game_math.h"
#include "animation.h"

typedef struct {
    int id;
    SDL_GameController* controller;
    Animator* animator;

    bool enabled;
    bool jumping;
    Vector2 moveDir;
    float triggerStrength;
    float deltaTrigger;
    Vector2 aim;
    uint32_t jumpStart;

    uint32_t jumpMaxTime;
    float armLength;
    float totalStrength;

    Vector2 position;
    float z;
    Vector2 velocity;
    float zVelocity;
} Player;

void PlayerInit(Player* player, int id);

int PlayerInput(Player* player, int keyboardIndex);
void PlayerUpdate(Player* player, double dt);
void PlayerRender(Player* player);
void PlayerRenderAim(Player* player);

bool PlayerIsOnGround(Player* player);
void PlayerSpike(Player* player);
Vector2 PlayerGetCenter(Player* player);


#endif // PLAYER_H