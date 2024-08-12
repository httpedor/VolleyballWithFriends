#ifndef PLAYER_H
#define PLAYER_H
#include <SDL.h>
#include "game_math.h"
#include "animation.h"

typedef enum {
    PLAYER_STATE_NONE, //parado ou andando
    PLAYER_STATE_BUMP, //fazendo manchete
    PLAYER_STATE_PANCAKE, //fazendo peixinho
    PLAYER_STATE_JUMP,
    PLAYER_STATE_SPIKE, //cortando
    PLAYER_STATE_SERVE, //sacando
} PlayerState;

typedef struct {
    int id; //qual jogador é
    SDL_GameController* controller; //qual controle
    Animator* animator; //como animar
    float whiteTint;

    int team; //qual time
    bool isServer; //se é o sacador

    bool enabled; //jogadores carregados 
    Vector2 moveDir;//direção
    Vector2 aim; // direção da mira 

    bool triggeredTint; //Se ja piscou
    float spikeStrength;
    uint32_t tossTime;
    uint32_t stateStart;
    PlayerState state;

    Vector2 pancakeTarget; //onde o peixinho ta mirando

    float armLength; //distancia da bola de encostar 
    float stamina; //stamina
    float staminaRender;

    Vector2 position; // em xy
    AABB collider; //pedro: 08/08(quase dia 9, é 23:50) eu to a tempos tentando fazer a colisão com a rede bonitinha, e não ta dando certo então eu vou usar a opção nuclear e usar aquelas funções de AABB que eu usei no meu RPG
    float z; //altura
    Vector2 velocity; //velocidade
    float zVelocity; // velocidade no ar

    Vector2 debug;

    uint32_t BUTTONS[SDL_CONTROLLER_BUTTON_MAX]; // a quanto tempo cada botão foi apertado
} Player;

void PlayerInit(Player* player, int id);

int PlayerInput(Player* player, int keyboardIndex);
void PlayerUpdate(Player* player, double dt);
void PlayerRender(Player* player);
void PlayerRenderAim(Player* player);

bool PlayerIsOnGround(Player* player);
void PlayerSpike(Player* player);
Vector2 PlayerGetCenter(Player* player);
int PlayerGetTeam(Player* player);

#endif // PLAYER_H