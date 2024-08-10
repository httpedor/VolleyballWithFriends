#include "player.h"
#include "camera.h"
#include "game.h"
#include <stdio.h>

#define DEBUG

const Vector2 PLAYER_SIZE = {28, 36};

void PlayerInit(Player* p, int id)
{
    p->id = id;
    p->controller = NULL;
    p->enabled = false;
    p->position.x = ((id % 2) == 0) ? -30 : 30;
    p->position.y = 10;

    p->velocity.x = 0;
    p->velocity.y = 0;

    p->aim = (Vector2){0, 0};
    p->jumpStart = 0;

    p->jumpMaxTime = 250;
    p->totalStrength = 20;
    p->armLength = PIXELS_PER_METER * 0.6;
    p->animator = AnimatorCreate();

    p->collider = (AABB){p->position.x, p->position.y, PLAYER_SIZE.x-5, PLAYER_SIZE.y};

    Animation* idle = AnimationCreate("player/playerIdle.bmp", (Vector2){32, 43});
    idle->fps = 18;
    Animation* walkAnim = AnimationCreate("player/playerRun.bmp", (Vector2){32, 43});
    walkAnim->fps = 18;

    AnimatorAddAnimation(p->animator, idle);
    AnimatorAddAnimation(p->animator, walkAnim);
}


int PlayerInput(Player* p, int notWithController)
{
    Vector2 movement;
    bool jumping = false;
    float trigger = 0;
    Vector2 aim;

    if (p->controller == NULL)
    {
        if (notWithController == 0)
        {
            aim = Vector2Sub(GetMousePosWorld(), PlayerGetCenter(p));
            movement.x = GameIsKeyDown(SDL_SCANCODE_D) - GameIsKeyDown(SDL_SCANCODE_A);
            movement.y = GameIsKeyDown(SDL_SCANCODE_W) - GameIsKeyDown(SDL_SCANCODE_S);
            trigger = GameIsKeyDown(SDL_SCANCODE_E) - GameIsKeyDown(SDL_SCANCODE_Q);
            if (GameIsKeyDown(SDL_SCANCODE_SPACE))
                jumping = true;
            if (GameIsKeyDown(SDL_SCANCODE_F))
                PlayerSpike(p);
            if (GameIsKeyDown(SDL_SCANCODE_R))
            {
                GameGetBall()->position = p->position;
                p->debug = GameGetBall()->position;
                GameGetBall()->z = 40;
            }
        }
        else if (notWithController == 1)
        {
            aim = p->aim;
            aim.x += GameIsKeyDown(SDL_SCANCODE_KP_6) - GameIsKeyDown(SDL_SCANCODE_KP_4) + GameIsKeyDown(SDL_SCANCODE_KP_9) - GameIsKeyDown(SDL_SCANCODE_KP_7) + GameIsKeyDown(SDL_SCANCODE_KP_3) - GameIsKeyDown(SDL_SCANCODE_KP_1);
            aim.y += GameIsKeyDown(SDL_SCANCODE_KP_8) - GameIsKeyDown(SDL_SCANCODE_KP_2) + GameIsKeyDown(SDL_SCANCODE_KP_9) - GameIsKeyDown(SDL_SCANCODE_KP_3) + GameIsKeyDown(SDL_SCANCODE_KP_7) - GameIsKeyDown(SDL_SCANCODE_KP_1);
            if (GameIsKeyDown(SDL_SCANCODE_KP_5))
                aim = (Vector2){0, 0};
            aim = Vector2Normalize(aim);
            movement.x = GameIsKeyDown(SDL_SCANCODE_RIGHT) - GameIsKeyDown(SDL_SCANCODE_LEFT);
            movement.y = GameIsKeyDown(SDL_SCANCODE_UP) - GameIsKeyDown(SDL_SCANCODE_DOWN);
            trigger = GameIsKeyDown(SDL_SCANCODE_KP_PLUS) - GameIsKeyDown(SDL_SCANCODE_KP_MINUS);
            if (GameIsKeyDown(SDL_SCANCODE_KP_0))
                jumping = true;
            if (GameIsKeyDown(SDL_SCANCODE_DOWN))
                PlayerSpike(p);
        }
        notWithController++;
    }
    else
    {
        SDL_GameController* con = p->controller;
        const float AXIS_MAX = 32768.0f;
        movement.x = SDL_GameControllerGetAxis(con, SDL_CONTROLLER_AXIS_LEFTX) / AXIS_MAX;
        movement.y = SDL_GameControllerGetAxis(con, SDL_CONTROLLER_AXIS_LEFTY) / AXIS_MAX;
        jumping = SDL_GameControllerGetButton(con, SDL_CONTROLLER_BUTTON_A);
        trigger = SDL_GameControllerGetAxis(con, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) / AXIS_MAX;
        trigger -= SDL_GameControllerGetAxis(con, SDL_CONTROLLER_AXIS_TRIGGERLEFT) / AXIS_MAX;
        aim = (Vector2){SDL_GameControllerGetAxis(con, SDL_CONTROLLER_AXIS_RIGHTX)/AXIS_MAX, SDL_GameControllerGetAxis(con, SDL_CONTROLLER_AXIS_RIGHTY)/AXIS_MAX};
        if (SDL_GameControllerGetButton(con, SDL_CONTROLLER_BUTTON_X))
            PlayerSpike(p);
    }


    if (!p->jumping && jumping && PlayerIsOnGround(p))
    {
        p->jumpStart = SDL_GetTicks();
        p->velocity.y = 8 * PIXELS_PER_METER;
    }
    p->jumping = jumping;
    p->moveDir = Vector2Normalize((Vector2){SDL_clamp(movement.x, -1, 1), SDL_clamp(movement.y, -1, 1)});
    p->deltaTrigger = SDL_clamp(trigger, -1, 1);
    p->aim = Vector2Normalize(aim);

    return notWithController;
}

void PlayerUpdate(Player* p, double dt) //fisica do jogador
{
    float maxMoveSpeed = 6.5 * PIXELS_PER_METER;


    if (p->jumping && SDL_GetTicks() - p->jumpStart <= p->jumpMaxTime)
    {
        p->zVelocity += 10 * PIXELS_PER_METER * dt;
    }
    else
    {
        p->zVelocity -= 20 * PIXELS_PER_METER * dt;
        p->velocity.x = p->moveDir.x * maxMoveSpeed;
        p->velocity.y = p->moveDir.y * maxMoveSpeed;
    }

    p->position.x += p->velocity.x * dt;
    p->position.y += p->velocity.y * dt;
    p->z += p->zVelocity * dt;

    p->collider.x = p->position.x - p->collider.w/2;
    p->collider.y = p->position.y + p->collider.h/2;

    float triggerChange = p->deltaTrigger * dt * 200;
    p->triggerStrength = SDL_clamp(p->triggerStrength + triggerChange, 0, 100);

    if (p->z < 0) {
        p->z = 0;
        p->zVelocity = 0;
    }

    /* pedor: CODIGO ANTIGO DE COLISÃO COM A REDE
    if (oldPos.x <= -15 && p->position.x > -25)
        p->position.x = -14;
    if (oldPos.x >= 15 && p->position.x < 25)
        p->position.x = 14;
        printf("%f \n", p->position.x);
    
    if (p->position.x >= -15)
        p->position.x = -30; //vitor: FAIXA DE VALORES ERA O PROBLEMA
        //vitor: falar com o pedro sobre o seguinte, o B.O. ali era a faixa de valores
        // agora como vamos organizar é outros 500
    */
    // pedro: CODIGO NOVO DE COLISÃO COM A REDE
    Vector2 intersectionPoint;
    if (AABBLineIntersection(p->collider, GameGetData()->net, &intersectionPoint))
    {
        bool isLeft = PointLeftOfLine(GameGetData()->net, p->position);
        p->debug = intersectionPoint;
        //pedro: Olha, eu ja fiz muito codigo feio e gambiarra, mas isso aqui é de outro nivel
        //isso usa o fato que a gente sabe que o jogador só vai colidir com a rede e a gente sabe que a rede é vertical
        p->position.x -= ((PLAYER_SIZE.x/2.0f) - SDL_fabs(p->position.x - intersectionPoint.x)) * (isLeft ? 1 : -1);
    }

    if (p->position.y >= 100)
        p->position.y = 100;

    if (p->position.y <= -160)
        p->position.y = -160;

    if (p->position.x <= -330)
        p->position.x = -330;

    if (p->position.x >= 300)
        p->position.x = 300;

    //Make the player sprite face the direction of movement
    if (p->velocity.x > 0)
        p->animator->flipped = false;
    else if (p->velocity.x < 0)
        p->animator->flipped = true;

    //Walking animation
    if (Vector2Length(p->velocity) > 0)
        AnimatorSetCurrentAnimationIndex(p->animator, 1);
    else
        AnimatorSetCurrentAnimationIndex(p->animator, 0);

    AnimatorUpdate(p->animator, dt);
}

void PlayerRender(Player* p)
{
    Vector2 projectedPosition = Vector2Add(Vector2Add(p->position, (Vector2){-PLAYER_SIZE.x/2, PLAYER_SIZE.y/2}), (Vector2){0, p->z});
    CameraRenderAnimator(GameGetCamera(), p->animator, projectedPosition, PLAYER_SIZE);
    if (p->triggerStrength != 0)
    {
        const float barHeight = 25;
        const float barWidth = 100;
        Rect2D maxStrength = {p->position.x - barWidth/4, p->position.y + 43 + barHeight + 10, barWidth, barHeight};
        Rect2D currentStrength = {maxStrength.x, maxStrength.y, barWidth/100 * p->triggerStrength, barHeight};
        SDL_SetRenderDrawColor(GameGetData()->renderer, 0x00, 0xff, 0x00, 0xff);
        CameraRenderRectOutlineF(GameGetCamera(), maxStrength, GameGetData()->renderer);
        CameraRenderRectF(GameGetCamera(), currentStrength, GameGetData()->renderer);
    }
    #ifdef DEBUG
    //Godot my beloved que saudade
    SDL_SetRenderDrawBlendMode(GameGetData()->renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(GameGetData()->renderer, 0x00, 0xff, 0xff, 0x66);
    CameraRenderRect(GameGetCamera(), p->collider, GameGetData()->renderer);
    SDL_SetRenderDrawColor(GameGetData()->renderer, 0xff, 0x00, 0xff, 0x66);
    CameraRenderRect(GameGetCamera(), (Rect2D){p->position.x, p->position.y, 2, 2}, GameGetData()->renderer);
    SDL_SetRenderDrawColor(GameGetData()->renderer, 0xff, 0x00, 0x00, 0x66);
    CameraRenderCircle(GameGetCamera(), p->debug, 3, GameGetData()->renderer);
    SDL_SetRenderDrawBlendMode(GameGetData()->renderer, SDL_BLENDMODE_NONE);
    #endif
}

void PlayerRenderAim(Player* p)
{
    if (p->aim.x != 0 || p->aim.y != 0)
    {
        SDL_SetRenderDrawColor(GameGetData()->renderer, 0xff, 0xff, 0xff, 0x33);
        for (int i = 1; i <= 3; i++)
        {
            CameraRenderCircle(GameGetCamera(), Vector2Add(PlayerGetCenter(p), Vector2Mul(p->aim, i*PIXELS_PER_METER/2)), 5*i, GameGetData()->renderer);
        }
    }
}

bool PlayerIsOnGround(Player* p)
{
    return p->position.y <= 0;
}

Vector2 PlayerGetCenter(Player* p)
{
    return (Vector2){p->position.x + PLAYER_SIZE.x/2, p->position.y + PLAYER_SIZE.y/2};
}

void PlayerSpike(Player* p)
{
    if (Vector2Distance(GameGetBall()->position, PlayerGetCenter(p)) < p->armLength)
    {
        GameGetBall()->velocity = Vector2Mul(p->aim, (float)p->totalStrength * (p->triggerStrength/100.0f) * PIXELS_PER_METER);
    }
}