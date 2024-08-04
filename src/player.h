#ifndef PLAYER_H
#define PLAYER_H
#include <SDL.h>
#include "game_math.h"
#include "animation.h"

typedef struct {
    int id; //qual jogador é
    SDL_GameController* controller; //qual controle
    Animator* animator; //como animar

    bool enabled; //jogadores carregados 
    bool jumping; //apertar espaço pra ser true 
    Vector2 moveDir;//direção
    float triggerStrength; //barra de força 0 a 1 ou 100
    float deltaTrigger; // quanto a barra muda por frame 
    Vector2 aim; // direção da mira 
    uint32_t jumpStart; // qual ms o jogador starta o pulo 

    uint32_t jumpMaxTime; //tempo o pulo continua subindo
    float armLength; //distancia da bola de encostar 
    float totalStrength;//max do triggerstrength

    Vector2 position; // em xy
    float z; //altura
    Vector2 velocity; //velocidade
    float zVelocity; // velocidade no ar
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