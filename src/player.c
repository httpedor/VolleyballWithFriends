#include "player.h"
#include "camera.h"
#include "game.h"
#include <stdio.h>
#include <stdlib.h>

//#define DEBUG

const Vector2 PLAYER_SIZE = {28, 36};
const uint32_t PERFECT_BUMP_DURATION = 1000;
const uint32_t PERFECT_SPIKE_DURATION = 1000;
const uint32_t MIN_BUMP_FOR_PASS = 250;
const uint32_t MAX_STAR_PASS_DURATION = 250;
const uint32_t PERFECT_UNDERSERVE_DURATION = 1500;
const uint32_t PERFECT_OVERSERVE_DURATION = 925;
const float BUMP_STAMINA_RECOVERY = 25;

void PlayerInit(Player* p, int id)
{
    p->id = id;
    p->team = (id % 2 == 0) ? TEAM_LEFT : TEAM_RIGHT;
    p->controller = NULL;
    p->enabled = false;
    p->position.x = ((id % 2) == 0) ? -30 : 30;
    p->position.y = 10;

    p->whiteTint = 0;
    p->stamina = 0;

    p->stateStart = 0;
    p->state = PLAYER_STATE_NONE;

    p->velocity.x = 0;
    p->velocity.y = 0;

    p->aim = (Vector2){0, 0};

    p->armLength = PIXELS_PER_METER * 1.5;
    p->animator = AnimatorCreate();

    p->collider = (AABB){p->position.x, p->position.y, PLAYER_SIZE.x-5, PLAYER_SIZE.y};

    Animation* idle = AnimationCreate("player/playerIdle.bmp", (Vector2){32, 43});
    idle->fps = 18;
    Animation* walkAnim = AnimationCreate("player/playerRun.bmp", (Vector2){32, 43});
    walkAnim->fps = 18;
    Animation* receptionAnim = AnimationCreate("player/playerReception.bmp", (Vector2){32, 43});
    receptionAnim->fps = 18;
    receptionAnim->loop = false;
    receptionAnim->lastFrame = 5;
    Animation* pancakeAnim = AnimationCreate("player/playerSlide.bmp", (Vector2){43, 43});
    pancakeAnim->fps = 18;
    pancakeAnim->loop = false;
    Animation* serveAnim = AnimationCreate("player/playerServe.bmp", (Vector2){32, 43});
    serveAnim->fps = 18;
    serveAnim->lastFrame = 3;
    serveAnim->loop = false;
    Animation* spikeAnim = AnimationCreate("player/playerSmash.bmp", (Vector2){32, 50});
    spikeAnim->fps = 18;
    spikeAnim->loop = false;
    spikeAnim->firstFrame = 8;
    spikeAnim->lastFrame = 12;
    Animation* jumpAnim = AnimationCreate("player/playerSmash.bmp", (Vector2){32, 43});
    jumpAnim->fps = 18;
    jumpAnim->loop = false;
    jumpAnim->lastFrame = 5;

    AnimatorAddAnimation(p->animator, idle);
    AnimatorAddAnimation(p->animator, walkAnim);
    AnimatorAddAnimation(p->animator, receptionAnim);
    AnimatorAddAnimation(p->animator, pancakeAnim);
    AnimatorAddAnimation(p->animator, serveAnim);
    AnimatorAddAnimation(p->animator, spikeAnim);
    AnimatorAddAnimation(p->animator, jumpAnim);

    for (int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; i++)
        p->BUTTONS[i] = 0;
}

void PlayerTryChangeState(Player* p, PlayerState newState)
{
    //Nunca trocar de estado se estiver no meio de um pulo
    if (p->state == PLAYER_STATE_JUMP && newState != PLAYER_STATE_SPIKE && newState != PLAYER_STATE_SERVE)
        return;
    //Se preparando ataque, só trocar de estado se for pra cancelar ou atacar.
    if (p->state == PLAYER_STATE_SERVE && newState != PLAYER_STATE_NONE && newState != PLAYER_STATE_SPIKE && newState != PLAYER_STATE_JUMP)
        return;

    //Nunca trocar de estado se estiver sacando e a bola não foi sacada
    if (p->state == PLAYER_STATE_SERVE && p->isServer && !GameGetBall()->isServed && newState != PLAYER_STATE_SPIKE && newState != PLAYER_STATE_JUMP)
        return;

    //Nunca dar double jump
    if (newState == PLAYER_STATE_JUMP && !PlayerIsOnGround(p))
        return;

    //Nunca trocar de estado se estiver no meio de um peixinho
    if (p->state == PLAYER_STATE_PANCAKE && (SDL_GetTicks() - p->stateStart) < AnimatorGetCurrentAnimation(p->animator)->frameCount * (1.0/AnimatorGetCurrentAnimation(p->animator)->fps) * 1000)
        return;
    //Nunca trocar de estado se estiver no meio de uma animação de ataque
    if (p->state == PLAYER_STATE_SPIKE && (SDL_GetTicks() - p->stateStart) < AnimatorGetCurrentAnimation(p->animator)->frameCount * (1.0/AnimatorGetCurrentAnimation(p->animator)->fps) * 1000)
        return;

    //Não atualizar o estado se for o mesmo
    if (p->state == newState)
        return;
    
    PlayerForceChangeState(p, newState);
}

void PlayerForceChangeState(Player* p, PlayerState newState)
{
    p->state = newState;
    p->stateStart = SDL_GetTicks();
    p->triggeredTint = false;
}

int PlayerInput(Player* p, int notWithController)
{
    Vector2 movement;
    bool jumping = false;
    bool bumping = false;
    bool spiking = false;
    bool pancaking = false;
    Vector2 pancakeDirection;
    Vector2 aim;

    if (p->controller == NULL)
    {
        if (notWithController == 0)
        {
            movement.x = GameIsKeyDown(SDL_SCANCODE_D) - GameIsKeyDown(SDL_SCANCODE_A);
            movement.y = GameIsKeyDown(SDL_SCANCODE_W) - GameIsKeyDown(SDL_SCANCODE_S);
            aim = movement;
            if (GameIsKeyDown(SDL_SCANCODE_SPACE))
                jumping = true;
            if (GameIsKeyDown(SDL_SCANCODE_J))
                spiking = true;
            if (GameIsKeyDown(SDL_SCANCODE_K))
                bumping = true;
            if (GameIsKeyDown(SDL_SCANCODE_L) && Vector2Length(movement) != 0)
            {
                pancaking = true;
                pancakeDirection = movement;
            }

            if (GameIsKeyDown(SDL_SCANCODE_R))
            {
                GameSetupRally(p->team);
            }
        }
        else if (notWithController == 1)
        {
            movement.x = GameIsKeyDown(SDL_SCANCODE_RIGHT) - GameIsKeyDown(SDL_SCANCODE_LEFT);
            movement.y = GameIsKeyDown(SDL_SCANCODE_UP) - GameIsKeyDown(SDL_SCANCODE_DOWN);
            aim = movement;
            if (GameIsKeyDown(SDL_SCANCODE_KP_0))
                jumping = true;
            if (GameIsKeyDown(SDL_SCANCODE_KP_2))
                bumping = true;
            if (GameIsKeyDown(SDL_SCANCODE_KP_3) && Vector2Length(movement) > 0)
            {
                pancaking = true;
                pancakeDirection = movement;
            }
            if (GameIsKeyDown(SDL_SCANCODE_KP_1))
                spiking = true;
        }
        notWithController++;
    }
    else
    {
        SDL_GameController* con = p->controller;

        const float AXIS_MAX = SDL_JOYSTICK_AXIS_MAX;
        movement.x = SDL_GameControllerGetAxis(con, SDL_CONTROLLER_AXIS_LEFTX) / AXIS_MAX;
        movement.y = -SDL_GameControllerGetAxis(con, SDL_CONTROLLER_AXIS_LEFTY) / AXIS_MAX;
        if (SDL_fabsf(movement.x) < GameGetConfig()->deadzoneLeft)
            movement.x = 0;
        if (SDL_fabsf(movement.y) < GameGetConfig()->deadzoneLeft)
            movement.y = 0;

        jumping = SDL_GameControllerGetButton(con, SDL_CONTROLLER_BUTTON_A);
        if (SDL_GameControllerGetButton(con, SDL_CONTROLLER_BUTTON_B))
            bumping = true;


        pancakeDirection = (Vector2){SDL_GameControllerGetAxis(con, SDL_CONTROLLER_AXIS_RIGHTX)/AXIS_MAX, -SDL_GameControllerGetAxis(con, SDL_CONTROLLER_AXIS_RIGHTY)/AXIS_MAX};

        if (SDL_fabsf(pancakeDirection.x) < GameGetConfig()->deadzoneRight)
            pancakeDirection.x = 0;
        if (SDL_fabsf(pancakeDirection.y) < GameGetConfig()->deadzoneRight)
            pancakeDirection.y = 0;

        if (Vector2Length(pancakeDirection) > 0)
            pancaking = true;


        aim = movement;
        if (SDL_GameControllerGetButton(con, SDL_CONTROLLER_BUTTON_X))
            spiking = true;
    }


    p->moveDir = Vector2Normalize((Vector2){SDL_clamp(movement.x, -1, 1), SDL_clamp(movement.y, -1, 1)});
    p->aim = Vector2Normalize(aim);
    if (jumping)
    {
        PlayerTryChangeState(p, PLAYER_STATE_JUMP);
    }
    else if (pancaking)
    {
        Vector2 targetPos = Vector2Add(p->position, Vector2Mul(p->moveDir, PIXELS_PER_METER * 4));
        if (Vector2Distance(GameGetBall()->targetPosition, targetPos) < PIXELS_PER_METER * 4)
        {
            targetPos = GameGetBall()->position;
        }
        p->pancakeTarget = targetPos;
        PlayerTryChangeState(p, PLAYER_STATE_PANCAKE);
    }
    else if (bumping)
        PlayerTryChangeState(p, PLAYER_STATE_BUMP);
    else if (spiking)
    {
        if (PlayerIsOnGround(p))
        {
            if (GameGetBall()->isServed)
                PlayerTryChangeState(p, PLAYER_STATE_SERVE);
            else if (p->isServer && GameGetData()->lastPlayerToTouchBall != p->id)
            {
                if (p->tossTime == 0)
                {
                    p->tossTime = SDL_GetTicks();
                    GameTouchBall(p->id, p->position, 1.5, PIXELS_PER_METER * 4, PATH_PARABOLIC, false, 0);
                    GameGetBall()->startPosition = p->position;
                }
                else if (SDL_GetTicks() - p->tossTime > 100)
                {
                    PlayerTryChangeState(p, PLAYER_STATE_SPIKE);
                }
            }
        }
        else
            PlayerTryChangeState(p, PLAYER_STATE_SPIKE);
    }
    else
        PlayerTryChangeState(p, PLAYER_STATE_NONE);

    return notWithController;
}

void PlayerUpdate(Player* p, double dt) //fisica do jogador
{
    p->whiteTint = Lerp(p->whiteTint, 0, dt * 5);
    if (p->whiteTint < 0.01)
        p->whiteTint = 0;

    float maxMoveSpeed = 5.75 * PIXELS_PER_METER;
    uint32_t timeInState = SDL_GetTicks() - p->stateStart;
    Ball* ball = GameGetBall();
    switch (p->state)
    {
    case PLAYER_STATE_NONE:
    {
        if (Vector2Length(p->velocity) > 0)
            AnimatorSetCurrentAnimationIndex(p->animator, 1); //Walking animation
        else
            AnimatorSetCurrentAnimationIndex(p->animator, 0); //Idle animation
        break;
    }
    case PLAYER_STATE_BUMP:
    {
        AnimatorSetCurrentAnimationIndex(p->animator, 2); //Reception animation

        maxMoveSpeed = PIXELS_PER_METER;
        if (timeInState >= PERFECT_BUMP_DURATION && !p->triggeredTint)
        {
            p->whiteTint = 1;
            p->triggeredTint = true;
        }

        if (GameGetData()->lastPlayerToTouchBall == p->id)
            break;

        uint32_t trailColorLight = 0xFF660022;
        uint32_t trailColorMedium = 0xFF660033;
        uint32_t trailColorStrong = 0xFF660066;

        float xDiff = ball->position.x - p->position.x;
        float yDiff = ball->position.y - p->position.y;
        float zDiff = ball->z - p->z;
        float distance = SDL_sqrtf(xDiff*xDiff + yDiff*yDiff + zDiff*zDiff);
        if (distance < p->armLength)
        {
            Vector2 target;
            int touchCount = GameGetData()->teamTouchCount[p->team];
            int sideMultiplier = ((p->team == TEAM_LEFT) ? -1 : 1);
            bool isPerfect = timeInState >= PERFECT_BUMP_DURATION;
            bool isMedium = timeInState >= MIN_BUMP_FOR_PASS && timeInState < PERFECT_BUMP_DURATION;
            bool isBad = timeInState < MIN_BUMP_FOR_PASS;
            ball->isStarSet = false;
            Player* teammate = GameGetOtherPlayerInTeam(p->id);
            switch (touchCount)
            {
            case 0:
                //Pass to setter
                target = (Vector2){PIXELS_PER_METER * sideMultiplier, -PIXELS_PER_METER};
                if (GameGetBall()->isStarSpike && GameGetPlayer(GameGetData()->lastPlayerToTouchBall)->team != p->team)
                {
                    //se for ataque perfeito e o ultimo a tocar foi do outro time, tem que ser passe no timing perfeito tambem
                    if (timeInState >= MAX_STAR_PASS_DURATION)
                    {
                        target = p->position;
                        target.x += GameRandom(0, PIXELS_PER_METER) * sideMultiplier;
                        target.y += GameRandom(-PIXELS_PER_METER, PIXELS_PER_METER);
                        GameTouchBall(p->id, target, 0.5, PIXELS_PER_METER, PATH_LINEAR, true, trailColorLight);
                    }
                    else
                    {
                        GameTouchBall(p->id, target, 1.75, PIXELS_PER_METER * 8, PATH_PARABOLIC, true, trailColorStrong);
                        p->stamina += BUMP_STAMINA_RECOVERY;
                    }

                }
                else
                {
                    if (isBad)
                    {
                        //Só RNGesus sabe onde a bola vai
                        target.x = GameRandom(X_MIN, X_MAX);
                        target.y = GameRandom(Y_MIN, Y_MAX);
                        float time;
                        //Se passar pro proprio time foi rápido. Se passar pro outro vai lento.
                        if (PointLeftOfLine(GameGetData()->net, p->position) == PointLeftOfLine(GameGetData()->net, target))
                            time = 1.25;
                        else
                            time = 2;

                        GameTouchBall(p->id, target, time, PIXELS_PER_METER * 8, PATH_PARABOLIC, true, trailColorLight);
                    }
                    else if (isMedium)
                    {
                        //Passa pro +- pro levantador, um pouco rápido
                        target.x += GameRandom(0, PIXELS_PER_METER) * sideMultiplier;
                        target.y += GameRandom(-PIXELS_PER_METER, PIXELS_PER_METER);
                        GameTouchBall(p->id, target, 1.5, PIXELS_PER_METER * 8, PATH_PARABOLIC, true, trailColorMedium);
                        p->stamina += BUMP_STAMINA_RECOVERY/2;
                    }
                    else if (isPerfect)
                    {
                        //Passe A pro levantador.
                        GameTouchBall(p->id, target, 2.5, PIXELS_PER_METER * 8, PATH_PARABOLIC, true, trailColorStrong);
                        p->stamina += BUMP_STAMINA_RECOVERY;
                    }
                }
                break;
            case 1:
                //Set to spiker
                if (isBad)
                {
                    //Só passa pro outro time
                    target = (Vector2){PIXELS_PER_METER * -sideMultiplier * 2, -PIXELS_PER_METER};
                    GameTouchBall(p->id, target, 2, PIXELS_PER_METER * 6, PATH_PARABOLIC, true, trailColorLight);
                }
                else if (isMedium)
                {
                    //Levantamento +-
                    target = (Vector2){PIXELS_PER_METER * sideMultiplier, teammate->position.y};
                    target.x += GameRandom(0, PIXELS_PER_METER) * sideMultiplier;
                    target.y += GameRandom(-PIXELS_PER_METER*2, PIXELS_PER_METER*2);
                    float time = GameRandom(1.5, 2);
                    GameTouchBall(p->id, target, time, time*4*PIXELS_PER_METER, PATH_PARABOLIC, true, trailColorMedium);
                    p->stamina += BUMP_STAMINA_RECOVERY/2;
                }
                else if (isPerfect)
                {
                    //Levantamento perfeito (StarSet)
                    target = (Vector2){PIXELS_PER_METER * sideMultiplier, teammate->position.y};
                    GameTouchBall(p->id, target, 2, PIXELS_PER_METER * 6, PATH_PARABOLIC, true, trailColorStrong);
                    GameGetBall()->isStarSet = true;
                    p->stamina += BUMP_STAMINA_RECOVERY;
                }
                break;
            case 2:
                //Pass to other team
                target = (Vector2){PIXELS_PER_METER * -sideMultiplier + (p->moveDir.x), -PIXELS_PER_METER + (p->moveDir.y)};
                target.x += GameRandom(0, PIXELS_PER_METER) * sideMultiplier;
                target.y += GameRandom(-PIXELS_PER_METER*2, PIXELS_PER_METER*2);
                float time;
                if (isBad)
                    time = 2.5;
                else if (isMedium)
                {
                    p->stamina += BUMP_STAMINA_RECOVERY/2;
                    time = 2;
                }
                else if (isPerfect)
                {
                    time = 1.75;
                    p->stamina += BUMP_STAMINA_RECOVERY;
                }
                GameTouchBall(p->id, target, time, PIXELS_PER_METER * 6, PATH_PARABOLIC, true, trailColorLight);
                break;
            default:
                break;
            }
        }
        break;
    }
    case PLAYER_STATE_PANCAKE:
    {
        AnimatorSetCurrentAnimationIndex(p->animator, 3); //pancake animation
        p->velocity.x = 0;
        p->velocity.y = 0;

        p->position = Vector2Lerp(p->position, p->pancakeTarget, dt * 2);
        float xDiff = p->position.x - GameGetBall()->position.x;
        float yDiff = p->position.y - GameGetBall()->position.y;
        float zDiff = p->z - GameGetBall()->z;
        float distance = SDL_sqrtf(xDiff*xDiff + yDiff*yDiff + zDiff*zDiff);
        if (distance < p->armLength && GameGetData()->lastPlayerToTouchBall != p->id)
        {
            int touchCount = GameGetData()->teamTouchCount[p->team];
            if (touchCount < 2)
            {
                float ballX = p->position.x + GameRandom(-PIXELS_PER_METER, PIXELS_PER_METER);
                if (ballX > X_MAX)
                    ballX = X_MAX;
                else if (ballX < X_MIN)
                    ballX = X_MIN;
                float ballY = p->position.y + GameRandom(-PIXELS_PER_METER, PIXELS_PER_METER);
                float height = GameRandom(4, 9);
                float time = height / 4;
                GameTouchBall(p->id, (Vector2){ballX, ballY}, time, height * PIXELS_PER_METER, PATH_PARABOLIC, true, 0xFF660022);
            }
            else
            {
                float ballX;
                float ballY;
                if (p->team == TEAM_LEFT)
                    ballX = GameRandom(X_MIN, -50);
                else
                    ballX = GameRandom(50, X_MAX);
                ballY = GameRandom(Y_MIN+25, Y_MAX-25);
                int height = GameRandom(6, 9);
                float time = height / 3;
                GameTouchBall(p->id, (Vector2){ballX, ballY}, time, height * PIXELS_PER_METER, PATH_PARABOLIC, true, 0xFF660022);
            }
        }

        if (timeInState >= AnimatorGetCurrentAnimation(p->animator)->frameCount * (1.0/AnimatorGetCurrentAnimation(p->animator)->fps) * 1000)
        {
            PlayerTryChangeState(p, PLAYER_STATE_NONE);
        }
        break;
    }
    case PLAYER_STATE_JUMP:
    {
        if (PlayerIsOnGround(p))
        {
            if (timeInState <= 200)
            {
                if (Vector2Length(p->velocity) > 0 || (p->isServer && !ball->isServed))
                {
                    AnimatorSetCurrentAnimationIndex(p->animator, 6); //jump animation
                    p->zVelocity = 10 * PIXELS_PER_METER;
                }
                else
                {
                    p->zVelocity = 5 * PIXELS_PER_METER;
                }
            }
            else
            {
                if (p->isServer && !ball->isServed)
                    PlayerForceChangeState(p, PLAYER_STATE_SERVE);
                else
                    PlayerForceChangeState(p, PLAYER_STATE_NONE);
            }
        }
        break;
    }
    case PLAYER_STATE_SERVE:
    {
        AnimatorSetCurrentAnimationIndex(p->animator, 4); //serve animation
        if (ball->isServed)
            maxMoveSpeed = PIXELS_PER_METER;
        else
            maxMoveSpeed = 0;
        if (timeInState >= PERFECT_SPIKE_DURATION && !p->triggeredTint && ball->isServed)
        {
            p->whiteTint = 1;
            p->triggeredTint = true;
        }
        float xDiff = ball->position.x - p->position.x;
        float yDiff = ball->position.y - p->position.y;
        float zDiff = ball->z - p->z;
        float distance = SDL_sqrtf(xDiff*xDiff + yDiff*yDiff + zDiff*zDiff);
        if (distance <= p->armLength && GameGetData()->lastPlayerToTouchBall != p->id && ball->isServed)
        {
            float strength = 0;
            if (ball->isStarSet)
            {
                //Não importa o quão bem preparado foi o ataque, se foi star set é ataque perfeito
                strength = 1;
            }
            else
            {
                //Ataque normal
                if (timeInState >= PERFECT_SPIKE_DURATION)
                    strength = 1;
                else if (timeInState >= MIN_BUMP_FOR_PASS && timeInState < PERFECT_SPIKE_DURATION)
                    strength = 0.5;
                else
                    strength = 0;
            }

            PlayerTryChangeState(p, PLAYER_STATE_SPIKE);
            p->spikeStrength = strength;
        }
        break;
    }
    case PLAYER_STATE_SPIKE:
    {
        AnimatorSetCurrentAnimationIndex(p->animator, 5); //serve animation
        maxMoveSpeed = 0;
        
        //Ja bateu na bola, não bater de novo, se não a força do ataque seria baseado no fps
        if (GameGetData()->lastPlayerToTouchBall == p->id)
            break;

        Vector2 target = Vector2Add(p->team == TEAM_LEFT ? (Vector2){120, -20} : (Vector2){-120, -20}, Vector2Mul(p->aim, PIXELS_PER_METER * 3));
        target.x += GameRandom(-PIXELS_PER_METER, PIXELS_PER_METER);
        target.y += GameRandom(-PIXELS_PER_METER, PIXELS_PER_METER);
        uint32_t hitTime = SDL_GetTicks() - p->tossTime;

        float xDiff = ball->position.x - p->position.x;
        float yDiff = ball->position.y - p->position.y;
        float zDiff = ball->z - p->z;
        float distance = SDL_sqrtf(xDiff*xDiff + yDiff*yDiff + zDiff*zDiff);
        if (distance > p->armLength)
            break;

        if (!PlayerIsOnGround(p))
        {
            uint32_t trailColor = 0x7700C800;
            //Saque forte
            if (!ball->isServed && p->isServer)
            {
                uint32_t hitDiff = SDL_abs(PERFECT_OVERSERVE_DURATION - hitTime);
                p->spikeStrength = 1 - SDL_clamp((float)hitDiff / 500.0, 0, 1);
                printf("Time, Str: %d, %f\n", hitTime, p->spikeStrength);

                uint8_t alpha = (uint8_t)Lerp(0.0, 120.0, p->spikeStrength);
                trailColor &= 0xFFFFFF00;
                trailColor |= alpha;

                GameGetBall()->logShapeConstant = Lerp(1, 20, p->spikeStrength);
                GameTouchBall(p->id, target, Lerp(1.75, 0.75, p->spikeStrength), p->z + PIXELS_PER_METER, PATH_LOGARITHMIC, true, trailColor);
                GameGetBall()->isServed = true;
                break;
            }

            //Se não tiver no chão, só pode cortar StarSet, ou gastar stamina
            if (ball->isStarSet)
            {
                //Ataque perfeito independente do tempo. Não gastar stamina
                p->spikeStrength = 1;
            }
            else
            {
                //Gastar stamina, precisa acertar o tempo
                if (p->stamina < 50)
                {
                    p->spikeStrength = 0;
                }
                else
                {
                    p->spikeStrength = SDL_clamp((float)1 - (distance / p->armLength) + 0.1, 0, 1);
                    p->stamina -= 50;
                }
            }

            uint8_t alpha = (uint8_t)Lerp(0.0, 120.0, p->spikeStrength);
            trailColor &= 0xFFFFFF00;
            trailColor |= alpha;
            if (p->spikeStrength > 0.5)
                GameTouchBall(p->id, target, Lerp(1.5, 0.5, p->spikeStrength), PIXELS_PER_METER * Lerp(10, 5, p->spikeStrength), PATH_LINEAR, true, trailColor);
            else
                GameTouchBall(p->id, target, 2, PIXELS_PER_METER * 8, PATH_PARABOLIC, true, 0xAAAAAA66);
        }
        else
        {
            if (!ball->isServed && p->isServer)
            {
                uint32_t hitDiff = SDL_abs(PERFECT_UNDERSERVE_DURATION - hitTime);
                p->spikeStrength = 1 - SDL_clamp((float)hitDiff / 2000.0, 0, 1);
                GameGetBall()->isServed = true;
            }
            GameTouchBall(p->id, target, Lerp(2.25, 1.75, p->spikeStrength), PIXELS_PER_METER * Lerp(10, 5, p->spikeStrength), PATH_PARABOLIC, true, 0xAAAAAA66);
        }
        break;
    }
    default:
        break;
    }

    p->staminaRender = Lerpf(p->staminaRender, p->stamina, dt * 4);

    p->zVelocity -= 20 * PIXELS_PER_METER * dt;
    if (p->state != PLAYER_STATE_PANCAKE)
    {
        if (PlayerIsOnGround(p))
        {
            p->velocity.x = p->moveDir.x * maxMoveSpeed;
            p->velocity.y = p->moveDir.y * maxMoveSpeed;
        }
        else if (p->zVelocity < 0)
        {
            p->zVelocity -= 20 * PIXELS_PER_METER * dt;
            p->velocity.x = Lerp(p->velocity.x, 0, dt*2);
        }
        p->position.x += p->velocity.x * dt;
        p->position.y += p->velocity.y * dt;
        p->z += p->zVelocity * dt;
    }

    p->collider.x = p->position.x - p->collider.w/2;
    p->collider.y = p->position.y + p->collider.h/2;

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

    if (p->position.y >= Y_MAX)
        p->position.y = Y_MAX;

    if (p->position.y <= Y_MIN)
        p->position.y = Y_MIN;

    if (p->position.x <= X_MIN)
        p->position.x = X_MIN;

    if (p->position.x >= X_MAX)
        p->position.x = X_MAX;

    //Make the player sprite face the direction of movement
    if (p->velocity.x > 0)
        p->animator->flipped = false;
    else if (p->velocity.x < 0)
        p->animator->flipped = true;

    //Animations
    AnimatorUpdate(p->animator, dt);
}

void PlayerRender(Player* p)
{
    Vector2 projectedPosition = Vector2Add(Vector2Add(p->position, (Vector2){-PLAYER_SIZE.x/2, PLAYER_SIZE.y/2}), (Vector2){0, p->z});
    CameraRenderAnimator(GameGetCamera(), p->animator, projectedPosition, PLAYER_SIZE);
    if (p->whiteTint > 0)
    {
        //Esse TextureColorMod não funciona com valores acima de 255, então eu tenho que fazer isso
        SDL_SetTextureBlendMode(AnimatorGetCurrentAnimation(p->animator)->texture, SDL_BLENDMODE_ADD);
        SDL_SetTextureColorMod(AnimatorGetCurrentAnimation(p->animator)->texture, 255 * p->whiteTint, 255 * p->whiteTint, 255 * p->whiteTint);
        CameraRenderAnimator(GameGetCamera(), p->animator, projectedPosition, PLAYER_SIZE);
        SDL_SetTextureColorMod(AnimatorGetCurrentAnimation(p->animator)->texture, 255, 255, 255);
        SDL_SetTextureBlendMode(AnimatorGetCurrentAnimation(p->animator)->texture, SDL_BLENDMODE_BLEND);
    }
    SDL_SetRenderDrawColor(GameGetData()->renderer, 0x00, 0x00, 0xff, 0xff);
    Rect2D staminaBar = {projectedPosition.x, projectedPosition.y - PLAYER_SIZE.y - 5, PLAYER_SIZE.x, 3};
    CameraRenderRectOutlineF(GameGetCamera(), staminaBar, GameGetData()->renderer);
    staminaBar.w = p->stamina / 100 * PLAYER_SIZE.x;
    SDL_SetRenderDrawColor(GameGetData()->renderer, 0x00, 0xff, 0x00, 0xff);
    CameraRenderRectF(GameGetCamera(), staminaBar, GameGetData()->renderer);
    staminaBar.w = p->staminaRender / 100.0 * PLAYER_SIZE.x;
    SDL_SetRenderDrawColor(GameGetData()->renderer, 0x00, 0x00, 0xff, 0xff);
    CameraRenderRectF(GameGetCamera(), staminaBar, GameGetData()->renderer);
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
    return p->z <= 0.1;
}

Vector2 PlayerGetCenter(Player* p)
{
    return (Vector2){p->position.x + PLAYER_SIZE.x/2, p->position.y + PLAYER_SIZE.y/2};
}

int PlayerGetTeam(Player* p)
{
    return p->team;
}