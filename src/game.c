#include <SDL.h>
#include <stdio.h>
#include "game.h"
#include "camera.h"
#include <SDL_ttf.h>

//TODO: Jumping throws ball faster
//TODO: If the right stick is up, it's a set, if it's down, it's a spike
//TODO: If in the first touch, the pass is towards the setter in the net. If it's the second, it's a high set, if it's the third, it just passes the ball
//TODO: Underhand serve will just be "aim and press a button"
//TODO: Overhand serve will just be "aim and shoot"
//TODO: When serving travel and float, the game will have a green bar telling you the trigger strength for the perfect serve
//TODO: When serving float, the strength bar will grow faster, but the green bar will be smaller
//TODO: When serving travel, the strength bar will grow slower, so you have to throw the ball higher and have better timing

//TODO: When passing the ball, there will be a circle inside the player. When an outer circle hits the inner circle, you have to press a button to make the perfect pass.
//      You can only pass when the ball is inside the outer circle, but it will be shit if the outer circle doesn't hit the inner circle

#define DEBUG

GameData* data;

void GameQuit() {
    SDL_DestroyRenderer(data->renderer);
    SDL_DestroyWindow(data->window);
    SDL_Quit();

    free(data);
}

Player* GameGetPlayer(int index)
{
    if (index >= 0 && index < MAX_PLAYER_COUNT)
        return &data->players[index];
    return NULL;
}

Camera* GameGetCamera() {
    return &data->camera;
}

GameData* GameGetData() {
    return data;
}

Ball* GameGetBall()
{
    return &data->ball;
}

bool GameIsKeyDown(SDL_Scancode scancode) {
    return data->KEYS[scancode];
}

Vector2 GameGetScreenSize() {
    return data->screenSize;
}

Rect2D GameGetCameraRect() {
    return (Rect2D){data->camera.position.x, data->camera.position.y, data->screenSize.x / data->camera.zoom, data->screenSize.y / data->camera.zoom};
}

Vector2 GetMousePosScreen()
{
    SDL_Point mousePos;
    SDL_GetMouseState(&mousePos.x, &mousePos.y);
    return (Vector2){(float)mousePos.x, (float)mousePos.y};
}

Vector2 GetMousePosWorld()
{
    return ApplyCameraTransformV(&data->camera, ScreenToWorld(GetMousePosScreen()));
}

GameData* GameInit() {

    data = (GameData*)malloc(sizeof(GameData));

    data->deltaTime = 0;
    data->shouldQuit = 0;
    data->paused = 0;
    printf("Iniciando Janela\n");
    data->window = SDL_CreateWindow(
                    "JOGO DANIEL(ele eh gostoso[o daniel nao o jogo])",
                    SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                    640, 360,
                    SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
                    );
    data->screenSize = (Vector2){640, 360};
    if (data->window == NULL) {
        fprintf(stderr, "Não foi possível criar a janela do Windows: %s", SDL_GetError());
        return 0;
    }
    printf("Iniciando Renderizador\n");
    data->renderer = SDL_CreateRenderer(data->window, -1, SDL_RENDERER_ACCELERATED);
    if (data->renderer == NULL) {
        fprintf(stderr, "Não foi possível criar o renderizador: %s", SDL_GetError());
        return 0;
    }

    SDL_RenderSetLogicalSize(data->renderer, 640, 360);

    data->font = TTF_OpenFont("arial.ttf", 24);
    if (data->font == NULL) {
        fprintf(stderr, "Não foi possível carregar a fonte: %s", TTF_GetError());
        return 0;
    }

    for (int i = 0; i < SDL_NUM_SCANCODES; i++) {
        data->KEYS[i] = false;
    }

    SDL_Surface* surface = SDL_LoadBMP("background.bmp");
    if (surface == NULL) {
        fprintf(stderr, "Não foi possível carregar a imagem de fundo: %s", SDL_GetError());
        return 0;
    }
    data->background = SDL_CreateTextureFromSurface(data->renderer, surface);
    SDL_FreeSurface(surface);
    surface = SDL_LoadBMP("net.bmp");
    if (surface == NULL) {
        fprintf(stderr, "Não foi possível carregar a imagem da rede: %s", SDL_GetError());
        return 0;
    }
    data->net = SDL_CreateTextureFromSurface(data->renderer, surface);
    SDL_FreeSurface(surface);

    data->camera.position = (Vector2){ 0, 0};
    data->camera.zoom = 1;

    data->ball.position = (Vector2){0, 0 };
    data->ball.velocity = (Vector2){ 0, 0 };
    data->ball.radius = PIXELS_PER_METER * 0.5;
    data->ball.z = PIXELS_PER_METER * 5;
    data->ball.zVelocity = 0;
    data->ball.animator = AnimatorCreate();
    AnimatorAddAnimation(data->ball.animator, AnimationCreate("ball.bmp", (Vector2){15, 15}));
    AnimatorGetAnimation(data->ball.animator, 0)->fps = 18;

    for (int i = 0; i < MAX_PLAYER_COUNT; i++)
        PlayerInit(GameGetPlayer(i), i);

    GameGetPlayer(0)->enabled = true;
    //GameGetPlayer(1)->enabled = true;

    for (int i = 0; i < SDL_NumJoysticks(); i++)
    {
        if (SDL_IsGameController(i))
        {
            data->players[i].controller = SDL_GameControllerOpen(i);
            if (data->players[i].controller == NULL)
                fprintf(stderr, "Não foi possível abrir o controle %d: %s\n", i, SDL_GetError());
            else
                printf("Controlando jogador numero %d\n", i);
        }
    }
    return data;
}

void GameInput() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type)
        {
        case SDL_KEYDOWN:
        {
            if (e.key.repeat)
                break;
            data->KEYS[e.key.keysym.scancode] = true;
            break;
        }
        case SDL_KEYUP:
        {
            data->KEYS[e.key.keysym.scancode] = false;
            break;
        }
        case SDL_QUIT:
        {
            data->shouldQuit = 1;
            break;
        }
        case SDL_WINDOWEVENT:
        {
            if (e.window.event == SDL_WINDOWEVENT_RESIZED)
            {
                //Vector2 newSize = {e.window.data1, e.window.data2};
                //GameGetCamera()->zoom = SDL_min(newSize.x / data->screenSize.x, newSize.y / data->screenSize.y);
                //data->screenSize.x = e.window.data1;
                //data->screenSize.y = e.window.data2;
            }
            break;
        }
        case SDL_CONTROLLERDEVICEADDED:
        {
            for (int i = 0; i < MAX_PLAYER_COUNT; i++)
            {
                if (SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(data->players[i].controller)) == e.cdevice.which)
                    break;
                Player* p = GameGetPlayer(i);
                if (p->controller == NULL)
                {
                    p->controller = SDL_GameControllerOpen(e.cdevice.which);
                    if (p->controller == NULL)
                    {
                        fprintf(stderr, "Não foi possível abrir o controle %d: %s\n", i, SDL_GetError());
                    }
                    printf("Assigned controller to player %d\n", i);
                    if (!p->enabled && p->id >= 3)
                    {
                        p->enabled = true;
                        p->position.x = 0;
                        p->position.y = 10;
                    }
                    break;
                }
            }
            break;
        }
        case SDL_CONTROLLERDEVICEREMOVED:
        {
            for (int i = 0; i < MAX_PLAYER_COUNT; i++)
            {
                if (data->players[i].controller != NULL && SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(data->players[i].controller)) == e.cdevice.which)
                {
                    SDL_GameControllerClose(data->players[i].controller);
                    data->players[i].controller = NULL;
                    if (data->players[i].enabled && i >= 3)
                    {
                        data->players[i].enabled = false;
                        data->players[i].position.x = 0;
                        data->players[i].position.y = 10;
                    }
                    break;
                }
            }
            break;
        }
        default:
            break;
        }
    }

    if (GameIsKeyDown(SDL_SCANCODE_ESCAPE))
        data->shouldQuit = 1;

    int notWithController = 0;
    for (int i = 0; i < MAX_PLAYER_COUNT; i++)
    {
        Player* p = GameGetPlayer(i);
        if (!(p->enabled))
            continue;
        notWithController = PlayerInput(p, notWithController);
    }

    if (GameIsKeyDown(SDL_SCANCODE_ESCAPE))
        data->paused = !data->paused;
    if (GameIsKeyDown(SDL_SCANCODE_R))
    {
        data->ball.position = Vector2Add(PlayerGetCenter(GameGetPlayer(0)), (Vector2){0, PIXELS_PER_METER*2});
        data->ball.velocity = (Vector2){ 0, 0 };
    }
}

void GameProcess(double dt) {
    Ball* b = &data->ball;
    b->position.x += b->velocity.x * dt;
    b->position.y += b->velocity.y * dt;

    b->zVelocity -= 11.0f * PIXELS_PER_METER * (float)dt;

    if (b->z <= 0) {
        b->z = 0;
        b->zVelocity = SDL_fabsf(b->zVelocity * 0.8f);
        if (b->zVelocity < PIXELS_PER_METER)
            b->zVelocity = 0;
    }

    AnimatorUpdate(b->animator, dt);

    for (int i = 0; i < MAX_PLAYER_COUNT; i++)
    {
        Player* p = GameGetPlayer(i);
        if (!p->enabled)
            continue;

        PlayerUpdate(p, dt);
    }

}

void GameRender(double dt, double fps) {
    SDL_Renderer* renderer = data->renderer;
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);
    SDL_RenderClear(renderer);

    //CameraRenderTexture(&data->camera, data->background, (Vector2){-data->screenSize.x/2, data->screenSize.y/2}, (Vector2){data->screenSize.x, data->screenSize.y});
    CameraRenderTexture(&data->camera, data->background, (Vector2){-320, 180}, (Vector2){640, 360});
    //Vector2 netSize = (Vector2){data->screenSize.x * 0.1, data->screenSize.y * 0.9};
    //CameraRenderTexture(&data->camera, data->net, (Vector2){-netSize.x/2, netSize.y/2 - (netSize.y*0.05)}, netSize);
    CameraRenderTexture(&data->camera, data->net, (Vector2){-17, 145}, (Vector2){34, 320});

    SDL_SetRenderDrawColor(renderer, 0xff, 0x00, 0x00, 0xff);
    CameraRenderAnimator(&data->camera, data->ball.animator, data->ball.position, (Vector2){data->ball.radius * 2, data->ball.radius * 2});

    for (int i = 0; i < MAX_PLAYER_COUNT; i++)
    {
        SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
        Player* p = GameGetPlayer(i);
        if (!p->enabled)
            continue;

        PlayerRender(p);
    }

    #ifdef DEBUG
    int len = snprintf(NULL, 0, "FPS: %.0f", fps);
    char* str = (char*)malloc(len + 1);
    snprintf(str, len + 1, "FPS: %.0f", fps);
    SDL_Surface* surface = TTF_RenderText_Solid(GameGetData()->font, str, (SDL_Color){255, 255, 255, 255});
    SDL_Texture* texture = SDL_CreateTextureFromSurface(GameGetData()->renderer, surface);
    SDL_Rect rect = {0, 0, surface->w, surface->h};
    SDL_RenderCopy(GameGetData()->renderer, texture, NULL, &rect);
    SDL_FreeSurface(surface);   
    SDL_DestroyTexture(texture);
    free(str);

    /*
    len = snprintf(NULL, 0, "strength: %.4f", GameGetPlayer(0)->triggerStrength);
    str = (char*)malloc(len + 1);
    snprintf(str, len + 1, "strength: %.4f", GameGetPlayer(0)->triggerStrength);
    surface = TTF_RenderText_Solid(GameGetData()->font, str, (SDL_Color){255, 255, 255, 255});
    texture = SDL_CreateTextureFromSurface(GameGetData()->renderer, surface);
    rect = (SDL_Rect){0, 30, surface->w, surface->h};
    SDL_RenderCopy(GameGetData()->renderer, texture, NULL, &rect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
    free(str);
    */
    #endif

    SDL_RenderPresent(renderer);
}