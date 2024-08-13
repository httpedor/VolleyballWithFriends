#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "game.h"
#include "camera.h"
#include <SDL_ttf.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#ifdef WIN32
#include <io.h>
#define F_OK 0
#define access _access
#endif


typedef enum {
    GAME_STATE_MAINSCREEN,
    GAME_STATE_OPTIONS,
    GAME_STATE_CHARACTER_SELECTION,
    GAME_STATE_PLAYING,
    GAME_STATE_PAUSED,
} GameState;

GameData* data;
Config* config;
GameState currentState = GAME_STATE_MAINSCREEN;
uint8_t currentMenuOption = 0;
uint8_t maxMenuOptions = 0;

void ChangeState(GameState newState)
{
    currentState = newState;
    currentMenuOption = 0;
    switch (newState)
    {
    case GAME_STATE_OPTIONS:
        maxMenuOptions = 4;
        break;
    case GAME_STATE_MAINSCREEN:
        maxMenuOptions = 2;
        break;
    
    default:
        maxMenuOptions = 0;
        break;
    }

    if (currentState != GAME_STATE_PLAYING)
        GameGetData()->paused = true;
}

void ChooseButton()
{
    switch (currentState)
    {
    case GAME_STATE_MAINSCREEN:
    {
        switch (currentMenuOption)
        {
        case 0:
            ChangeState(GAME_STATE_CHARACTER_SELECTION);
            break;
        case 1:
            ChangeState(GAME_STATE_OPTIONS);
            break;
        case 2:
            data->shouldQuit = 1;
            break;
        default:
            break;
        }
        break;
    }
    case GAME_STATE_OPTIONS:
    {
        switch (currentMenuOption)
        {
        case 0:
            SDL_SetWindowFullscreen(data->window, (SDL_GetWindowFlags(data->window) & SDL_WINDOW_FULLSCREEN_DESKTOP) ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);
            break;
        case 4:
            ChangeState(GAME_STATE_MAINSCREEN);
            break;
        default:
            break;
        }
        break;
    }
    default:
        break;
    }
}

void SliderChange(int side)
{
    if (side == 0)
        return;
    if (currentState != GAME_STATE_OPTIONS)
        return;
    switch (currentMenuOption)
    {
    case 1:
        config->volume += 0.1 * side;
        if (config->volume < 0)
            config->volume = 0;
        if (config->volume > 1)
            config->volume = 1;
        break;
    case 2:
        config->deadzoneLeft += 0.1 * side;
        if (config->deadzoneLeft < 0)
            config->deadzoneLeft = 0;
        if (config->deadzoneLeft > 1)
            config->deadzoneLeft = 1;
        break;
    case 3:
        config->deadzoneRight += 0.1 * side;
        if (config->deadzoneRight < 0)
            config->deadzoneRight = 0;
        if (config->deadzoneRight > 1)
            config->deadzoneRight = 1;
        break;
    default:
        break;
    }
}

void RenderButton(char* text, int x, int y, bool selected)
{
    if (selected)
        SDL_SetRenderDrawColor(GameGetData()->renderer, 0xff, 0xB0, 0x00, 0xff);
    else
        SDL_SetRenderDrawColor(GameGetData()->renderer, 0xff, 0xff, 0xff, 0xff);
    SDL_Surface* surface = TTF_RenderText_Solid(GameGetData()->font, text, selected ? (SDL_Color){255, 176, 0, 255} : (SDL_Color){255, 255, 255, 255});
    SDL_Texture* texture = SDL_CreateTextureFromSurface(GameGetData()->renderer, surface);
    SDL_Rect rect = {x-surface->w/2, y-surface->h/2, surface->w, surface->h};
    SDL_RenderCopy(GameGetData()->renderer, texture, NULL, &rect);
    rect.x -= 5;
    rect.y -= 5;
    rect.w += 10;
    rect.h += 10;
    SDL_RenderDrawRect(GameGetData()->renderer, &rect);
    SDL_FreeSurface(surface);   
    SDL_DestroyTexture(texture);
}

void RenderSlider(char* text, int x, int y, float value, bool selected)
{
    if (selected)
        SDL_SetRenderDrawColor(GameGetData()->renderer, 0xff, 0xB0, 0x00, 0xff);
    else
        SDL_SetRenderDrawColor(GameGetData()->renderer, 0xff, 0xff, 0xff, 0xff);
    SDL_Surface* surface = TTF_RenderText_Solid(GameGetData()->font, text, selected ? (SDL_Color){255, 176, 0, 255} : (SDL_Color){255, 255, 255, 255});
    SDL_Texture* texture = SDL_CreateTextureFromSurface(GameGetData()->renderer, surface);
    SDL_Rect rect = {x - surface->w - 2, y - surface->h, surface->w, surface->h};
    SDL_RenderCopy(GameGetData()->renderer, texture, NULL, &rect);
    rect = (SDL_Rect){x + 2, y - surface->h, 200, surface->h};
    SDL_RenderDrawRect(GameGetData()->renderer, &rect);

    SDL_FreeSurface(surface);   
    SDL_DestroyTexture(texture);

    int len = snprintf(NULL, 0, "%.0f%%", value*100);
    char* str = (char*)malloc(len + 1);
    snprintf(str, len + 1, "%.0f%%", value*100);

    surface = TTF_RenderText_Solid(GameGetData()->font, str, selected ? (SDL_Color){255, 255, 255, 255} : (SDL_Color){255, 176, 0, 255});
    texture = SDL_CreateTextureFromSurface(GameGetData()->renderer, surface);
    SDL_Rect r2 = {rect.x + rect.w/2 - surface->w/2, y-rect.h, surface->w, surface->h};

    rect.w = (int)(value * 200.0);
    SDL_RenderFillRect(GameGetData()->renderer, &rect);

    SDL_RenderCopy(GameGetData()->renderer, texture, NULL, &r2);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
    free(str);
}

void GameQuit() {
    SDL_DestroyRenderer(data->renderer);
    SDL_DestroyWindow(data->window);
    SDL_Quit();

    FILE* f = fopen("config.cfg", "w");
    fprintf(f, "deadzoneLeft=%f\n", config->deadzoneLeft);
    fprintf(f, "deadzoneRight=%f\n", config->deadzoneRight);
    fprintf(f, "volume=%f\n", config->volume);

    free(config);
    free(data);
}

Player* GameGetPlayer(int index)
{
    if (index >= 0 && index < MAX_PLAYER_COUNT)
        return &data->players[index];
    return NULL;
}

Player* GameGetOtherPlayerInTeam(int index)
{
    for (int i = 0; i < MAX_PLAYER_COUNT; i++)
    {
        if (i == index)
            continue;

        Player* p = GameGetPlayer(i);
        if (p->team == GameGetPlayer(index)->team)
            return p;
    }
}

Camera* GameGetCamera() {
    return &data->camera;
}

GameData* GameGetData() {
    return data;
}

Config* GameGetConfig()
{
    return config;
}

Ball* GameGetBall()
{
    return &data->ball;
}

bool GameIsKeyDown(SDL_Scancode scancode) {
    return data->KEYS[scancode] > 0;
}

uint32_t GameMillisKeyPressed(SDL_Scancode scancode)
{
    if (data->KEYS[scancode] == 0)
        return 0;
    return SDL_GetTicks() - data->KEYS[scancode];
}

Vector2 GameGetScreenSize() {
    return data->screenSize;
}

Rect2D GameGetCameraRect() {
    return (Rect2D){data->camera.position.x, data->camera.position.y, data->screenSize.x / data->camera.zoom, data->screenSize.y / data->camera.zoom};
}

double GameGetTimeScale()
{
    return data->timeScale;
}

float GameRandom(float min, float max)
{
    return ((1.0*rand()) / RAND_MAX) * (max - min) + min;
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

Team GameOtherteam(Team team)
{
    return team == TEAM_LEFT ? TEAM_RIGHT : TEAM_LEFT;
}

void GameTouchBall(int player, Vector2 target, float duration, float height, PathType pathType, bool countTouch, uint32_t color)
{
    data->ball.targetPosition = target;
    data->ball.startPosition = data->ball.position;
    data->ball.pathDuration = duration;
    data->ball.pathHeight = height;
    data->ball.pathProgress = 0;
    data->ball.pathType = pathType;
    data->ball.trailColor = color;
    /*switch (pathType)
    {
    case PATH_LINEAR:
        data->ball.trailColor = 0x7700C844;
        break;
    case PATH_LOGARITHMIC:
        data->ball.trailColor = 0xAAAAAA44;
        break;
    case PATH_PARABOLIC:
        data->ball.trailColor = 0xFF660044;
    default:
        break;
    }*/
    if (countTouch)
    {
        data->lastPlayerToTouchBall = player;
        Player* p = GameGetPlayer(player);
        data->teamTouchCount[p->team]++;
        data->teamTouchCount[GameOtherteam(p->team)] = 0;
    }
}

void GameSetupRally(Team server)
{
    data->teamTouchCount[0] = 0;
    data->teamTouchCount[1] = 0;
    data->lastPlayerToTouchBall = -1;
    data->ball.isServed = false;
    data->ball.isStarSet = false;
    data->ball.isStarSpike = false;
    bool foundServer = false;
    for (int i = 0; i < MAX_PLAYER_COUNT; i++)
    {
        Player* p = GameGetPlayer(i);
        if (!p->enabled)
            continue;
        p->tossTime = 0;
        p->isServer = false;
        p->position = (Vector2){p->team == TEAM_LEFT ? -50 : 50 , -10};
        p->animator->flipped = p->team == TEAM_RIGHT;
        if (p->team == server && !foundServer)
        {
            Vector2 serverPos = (Vector2){p->team == TEAM_LEFT ? X_MIN + 10: X_MAX-10, -10};
            if (data->lastTeamToServe == p->team && data->teamLastServer[p->team] == p->id)
                p->isServer = true;
            else if (data->lastTeamToServe != p->team && data->teamLastServer[p->team] != p->id)
            {
                p->isServer = true;
                data->teamLastServer[p->team] = p->id;
            }

            if (p->isServer)
            {
                foundServer = true;
                p->position = serverPos;
                p->state = PLAYER_STATE_SERVE;
            }
        }
    }
    data->lastTeamToServe = server;
}

GameData* GameInit() {
    config = (Config*)malloc(sizeof(Config));
    if (access("config.cfg", F_OK) == 0)
    {
        FILE* f = fopen("config.cfg", "r");
        char line[128];
        while (fgets(line, 128, f) != NULL)
        {
            if (line[0] == '#')
                continue;
            char key[64];
            char value[64];
            sscanf(line, "%[^=]=%s", key, value);
            if (strcmp(key, "deadzoneLeft") == 0)
                config->deadzoneLeft = atof(value);
            if (strcmp(key, "deadzoneRight") == 0)
                config->deadzoneRight = atof(value);
            if (strcmp(key, "volume") == 0)
                config->volume = atof(value);
        }
    }
    else
    {
        config->deadzoneLeft = 0.2;
        config->deadzoneRight = 0.2;
        config->volume = 1;
    }

    data = (GameData*)malloc(sizeof(GameData));

    srand(time(NULL));

    data->started = false;
    data->deltaTime = 0;
    data->timeScale = 1;
    data->shouldQuit = 0;
    data->paused = 0;
    printf("Iniciando Janela\n");
    data->window = SDL_CreateWindow(
                    "Rock Lee x Gaara ao som de linkin park",
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

    //pedro: Originalmente eu não fazia isso e meu sistem de câmera fazia com que quem tivesse monitor maior visse mais do jogo
    //mas daí eu percebi que eu sei o tamanho do mapa e cabe tudo numa tela 16:9, então eu fiz isso
    //teve até o efeito das barrinhas laterias pretas, que eu achei bonito pq eu re-zerei Undertale esses dias
    SDL_RenderSetLogicalSize(data->renderer, 640, 360);

    //Originalmente eu ia usar a fonte de Undertale, mas como o SDL_RenderSetLogicalSize só escala o jogo sem anti-aliasing, até a fonte que é só uma arial parece que eh pixel art
    //obrigado SDL S2
    data->font = TTF_OpenFont("arial.ttf", 24);
    if (data->font == NULL) {
        fprintf(stderr, "Não foi possível carregar a fonte: %s", TTF_GetError());
        return 0;
    }

    for (int i = 0; i < SDL_NUM_SCANCODES; i++) {
        data->KEYS[i] = 0;
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
    data->netTexture = SDL_CreateTextureFromSurface(data->renderer, surface);
    SDL_FreeSurface(surface);
    data->net = (Line2D){(Vector2){-15, 98}, (Vector2){8, -137}};

    data->camera.position = (Vector2){ 0, 0};
    data->camera.zoom = 1;

    data->ball.trailColor = 0xffffffff;
    for (int i = 0; i < 10; i++)
    {
        data->ball.trail[i] = (Vector2){0, 0};
        data->ball.trailZ[i] = 0;
    }
    data->ball.isStarSet = false;
    data->ball.isStarSpike = false;
    data->ball.isServed = false;
    data->ball.position = (Vector2){0, 0};
    data->ball.radius = PIXELS_PER_METER * 0.5;
    data->ball.z = PIXELS_PER_METER * 5;
    data->ball.animator = AnimatorCreate();
    surface = SDL_LoadBMP("ball_cursor.bmp");
    if (surface == NULL) {
        fprintf(stderr, "Não foi possível carregar a imagem da mira da bola: %s", SDL_GetError());
        return 0;
    }
    data->ball.projection_texture = SDL_CreateTextureFromSurface(data->renderer, surface);
    SDL_FreeSurface(surface);
    AnimatorAddAnimation(data->ball.animator, AnimationCreate("ball.bmp", (Vector2){15, 15}));
    AnimatorGetAnimation(data->ball.animator, 0)->fps = 18;

    data->lastPlayerToTouchBall = -1;

    data->teamScore[0] = 0;
    data->teamScore[1] = 0;
    data->teamTouchCount[0] = 0;
    data->teamTouchCount[1] = 0;
    data->teamLastServer[0] = -1;
    data->teamLastServer[0] = -1;
    data->lastTeamToServe = -1;

    for (int i = 0; i < MAX_PLAYER_COUNT; i++)
    {
        PlayerInit(GameGetPlayer(i), i);
        GameGetPlayer(i)->enabled = false;
    }

    //printf("Iniciando controles\n");
    /*for (int i = SDL_NumJoysticks(); i >= 0; i--)
    {
        if (SDL_IsGameController(i) && i < MAX_PLAYER_COUNT)
        {
            data->players[i].controller = SDL_GameControllerOpen(i);
            if (data->players[i].controller == NULL)
                fprintf(stderr, "Não foi possível abrir o controle %d: %s\n", i, SDL_GetError());
            else
                printf("Controlando jogador numero %d\n", i);
        }
    }*/
    
    ChangeState(GAME_STATE_MAINSCREEN);

    return data;
}

void GameInput(double dt) {
    SDL_Event e;

    bool selectionUp = false;
    bool selectionDown = false;
    bool selectButton = false;
    bool selectBack = false;
    bool selectionRight = false;
    bool selectionLeft = false;

    while (SDL_PollEvent(&e)) {
        switch (e.type)
        {
        case SDL_KEYDOWN:
        {
            if (e.key.repeat)
                break;
            SDL_Scancode code = e.key.keysym.scancode;
            data->KEYS[code] = SDL_GetTicks();

            if (code == SDL_SCANCODE_UP)
                selectionUp = true;
            if (code == SDL_SCANCODE_DOWN)
                selectionDown = true;
            if (code == SDL_SCANCODE_RIGHT)
                selectionRight = true;
            if (code == SDL_SCANCODE_LEFT)
                selectionLeft = true;

            if (code == SDL_SCANCODE_RETURN || code == SDL_SCANCODE_KP_ENTER || code == SDL_SCANCODE_SPACE || code == SDL_SCANCODE_J || code == SDL_SCANCODE_1)
                selectButton = true;
            if (code == SDL_SCANCODE_ESCAPE || code == SDL_SCANCODE_BACKSPACE || code == SDL_SCANCODE_K || code == SDL_SCANCODE_2)
                selectBack = true;

            if (currentState == GAME_STATE_CHARACTER_SELECTION)
            {
                int notWithControllers = 0;
                for (int i = 0; i < MAX_PLAYER_COUNT; i++)
                {
                    if (data->players[i].controller != NULL)
                        continue;
                    if (data->players[i].isReady)
                    {
                        if (notWithControllers == 0 && code == SDL_SCANCODE_K)
                            data->players[i].isReady = false;
                        if (notWithControllers == 1 && code == SDL_SCANCODE_KP_2)
                            data->players[i].isReady = false;

                        notWithControllers++;
                        continue;
                    }
                    bool left = false;
                    bool right = false;
                    bool ready = false;
                    if (notWithControllers == 0)
                    {
                        if (code == SDL_SCANCODE_A)
                            left = true;
                        if (code == SDL_SCANCODE_D)
                            right = true;
                        if (code == SDL_SCANCODE_SPACE || code == SDL_SCANCODE_J)
                            ready = true;
                    }
                    else if (notWithControllers == 1)
                    {
                        if (code == SDL_SCANCODE_LEFT)
                            left = true;
                        if (code == SDL_SCANCODE_RIGHT)
                            right = true;
                        if (code == SDL_SCANCODE_KP_ENTER || code == SDL_SCANCODE_KP_1)
                            ready = true;
                    }

                    char names[4][64] = {"Default", "Bruninho", "Strawhat", "Deusniel"};
                    if (left)
                    {
                        for (int j = 0; j < 4; j++)
                        {
                            if (strcmp(data->players[i].skinName, names[j]) == 0)
                            {
                                strcpy(data->players[i].skinName, names[(j+3)%4]);
                                break;
                            }
                        }
                    }
                    if (right)
                    {
                        for (int j = 0; j < 4; j++)
                        {
                            if (strcmp(data->players[i].skinName, names[j]) == 0)
                            {
                                strcpy(data->players[i].skinName, names[(j+1)%4]);
                                break;
                            }
                        }
                    }
                    if (ready)
                    {
                        data->players[i].isReady = true;
                        data->players[i].enabled = true;
                        PlayerChooseSkin(&data->players[i]);
                    }
                    notWithControllers++;
                }
            }
            break;
        }
        case SDL_KEYUP:
        {
            data->KEYS[e.key.keysym.scancode] = 0;
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
            for (int i = MAX_PLAYER_COUNT-1; i >= 0; i--)
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
                    break;
                }
            }
            break;
        }
        case SDL_CONTROLLERDEVICEREMOVED:
        {
            for (int i = MAX_PLAYER_COUNT; i < MAX_PLAYER_COUNT; i++)
            {
                if (data->players[i].controller != NULL && SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(data->players[i].controller)) == e.cdevice.which)
                {
                    SDL_GameControllerClose(data->players[i].controller);
                    data->players[i].controller = NULL;
                    break;
                }
            }
            break;
        }
        case SDL_CONTROLLERBUTTONDOWN:
        {
            for (int i = 0; i < MAX_PLAYER_COUNT; i++)
            {
                if (data->players[i].controller != NULL && SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(data->players[i].controller)) ==  e.cbutton.which)
                {
                    data->players[i].BUTTONS[e.cbutton.button] = SDL_GetTicks();
                    if (currentState == GAME_STATE_CHARACTER_SELECTION)
                    {
                        if (data->players[i].isReady)
                        {
                            if (e.cbutton.button == SDL_CONTROLLER_BUTTON_B)
                                data->players[i].isReady = false;

                            break;
                        }
                        bool left = false;
                        bool right = false;
                        bool ready = false;
                        if (e.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_LEFT)
                            left = true;
                        if (e.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_RIGHT)
                            right = true;
                        if (e.cbutton.button == SDL_CONTROLLER_BUTTON_A)
                            ready = true;
                        
                        char names[4][64] = {"Default", "Bruninho", "Strawhat", "Deusniel"};
                        if (left)
                        {
                            for (int j = 0; j < 4; j++)
                            {
                                if (strcmp(data->players[i].skinName, names[j]) == 0)
                                {
                                    strcpy(data->players[i].skinName, names[(j+3)%4]);
                                    break;
                                }
                            }
                        }
                        if (right)
                        {
                            for (int j = 0; j < 4; j++)
                            {
                                if (strcmp(data->players[i].skinName, names[j]) == 0)
                                {
                                    strcpy(data->players[i].skinName, names[(j+1)%4]);
                                    break;
                                }
                            }
                        }
                        if (ready)
                        {
                            data->players[i].isReady = true;
                            data->players[i].enabled = true;
                            PlayerChooseSkin(&data->players[i]);
                        }
                    }
                    break;
                }
            }

            if (e.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_UP)
                selectionUp = true;
            if (e.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_DOWN)
                selectionDown = true;
            if (e.cbutton.button == SDL_CONTROLLER_BUTTON_A)
                selectButton = true;
            if (e.cbutton.button == SDL_CONTROLLER_BUTTON_B)
                selectBack = true;
            
            if (e.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_RIGHT)
                selectionRight = true;
            if (e.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_LEFT)
                selectionLeft = true;

            break;
        }
        case SDL_CONTROLLERBUTTONUP:
        {
            for (int i = 0; i < MAX_PLAYER_COUNT; i++)
            {
                if (data->players[i].controller != NULL && SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(data->players[i].controller)) ==  e.cbutton.which)
                {
                    data->players[i].BUTTONS[e.cbutton.button] = 0;
                    break;
                }
            }
            break;
        }
        default:
            break;
        }
    }

    if (selectionUp)
        currentMenuOption--;
    if (selectionDown)
        currentMenuOption++;

    if (currentMenuOption < 0)
        currentMenuOption = maxMenuOptions;
    if (currentMenuOption > maxMenuOptions)
        currentMenuOption = 0;

    if (selectButton)
        ChooseButton();
    
    if (selectionLeft)
        SliderChange(-1);
    if (selectionRight)
        SliderChange(1);
    if (currentState == GAME_STATE_CHARACTER_SELECTION)
    {
        bool allReady = true;
        for (int i = 0; i < MAX_PLAYER_COUNT; i++)
        {
            if (!data->players[i].isReady)
            {
                allReady = false;
                break;
            }
        }

        if (allReady)
        {
            ChangeState(GAME_STATE_PLAYING);
            GameSetupRally(TEAM_LEFT);
            data->paused = false;
        }
    }

    if (currentState == GAME_STATE_PLAYING)
    {
        int notWithController = 0;
        for (int i = 0; i < MAX_PLAYER_COUNT; i++)
        {
            Player* p = GameGetPlayer(i);
            if (!(p->enabled))
                continue;
            notWithController = PlayerInput(p, notWithController);
        }
    }
}

void GameProcess(double dt) {
    Ball* b = &data->ball;

    if (b->pathProgress < 1)
    {
        b->pathProgress += dt / b->pathDuration;
        if (b->pathProgress > 1)
            b->pathProgress = 1;
        float t = b->pathProgress;
        float h = b->pathHeight;
        float ballX = Lerpf(b->startPosition.x, b->targetPosition.x, t);
        float ballY = Lerpf(b->startPosition.y, b->targetPosition.y, t);
        float ballZ = 0;
        float c = b->logShapeConstant;
        
        //Viva cálculo 1! que eu reprovei mas só porquê eu fui burro e não porquê eu não sabia
        switch (b->pathType)
        {
        case PATH_PARABOLIC:
            ballZ = h * 4 * t * (1 - t);
            break;
        case PATH_LINEAR:
            ballZ = h * (1 - t);
            break;
        case PATH_LOGARITHMIC:
            ballZ = h * (logf(1 + c * (1-t)) / logf(1 + c));
            break;
        default:
            break;
        }

        b->position = (Vector2){ballX, ballY};
        b->z = ballZ;
    }

    AnimatorUpdate(b->animator, dt);

    for (int i = BALL_TRAIL_LENGTH-1; i > 0; i--)
    {
        b->trail[i] = b->trail[i-1];
        b->trailZ[i] = b->trailZ[i-1];
    }
    b->trail[0] = b->position;
    b->trailZ[0] = b->z;

    for (int i = 0; i < MAX_PLAYER_COUNT; i++)
    {
        Player* p = GameGetPlayer(i);
        if (!p->enabled)
            continue;

        PlayerUpdate(p, dt);
    }

}

void BallRender()
{
    Vector2 projectedBallPosition = (Vector2){data->ball.position.x, data->ball.position.y + data->ball.z};
    Vector2 size = (Vector2){data->ball.radius * 2, data->ball.radius * 2};
    for (int i = 0; i < BALL_TRAIL_LENGTH; i++)
    {
        if (data->ball.trailZ[i] == 0)
            continue;
        Vector2 projectedTrailPosition = (Vector2){data->ball.trail[i].x, data->ball.trail[i].y + data->ball.trailZ[i]};
        SDL_SetRenderDrawColor(data->renderer, (data->ball.trailColor >> 24) & 0xff, (data->ball.trailColor >> 16) & 0xff, (data->ball.trailColor >> 8) & 0xff, data->ball.trailColor & 0xff);
        SDL_SetRenderDrawBlendMode(data->renderer, SDL_BLENDMODE_BLEND);
        CameraRenderCircle(&data->camera, projectedTrailPosition, Lerpf(data->ball.radius-4, 0, ((float)i)/((float)BALL_TRAIL_LENGTH)), data->renderer);
        SDL_SetRenderDrawBlendMode(data->renderer, SDL_BLENDMODE_NONE);
        SDL_SetRenderDrawColor(data->renderer, 0xff, 0xff, 0xff, 0xff);
    }
    CameraRenderAnimator(&data->camera, data->ball.animator, (Vector2){projectedBallPosition.x-size.x/2, projectedBallPosition.y+size.y/2}, size);
}
void BallAimRender()
{
    Vector2 size = (Vector2){15, 15};
    CameraRenderTexture(&data->camera, data->ball.projection_texture, (Vector2){data->ball.targetPosition.x-size.x/2, data->ball.targetPosition.y+size.y/2}, size);
}

void GameRender(double dt, double fps) {
    SDL_Renderer* renderer = data->renderer;
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);
    SDL_RenderClear(renderer);
    switch (currentState)
    {
        case GAME_STATE_MAINSCREEN:
        {
            char* str = "Volleyball With Friends";
            SDL_Surface* surface = TTF_RenderText_Solid(GameGetData()->font, str, (SDL_Color){255, 255, 255, 255});
            SDL_Texture* texture = SDL_CreateTextureFromSurface(GameGetData()->renderer, surface);
            SDL_Rect rect = {GameGetScreenSize().x/2-surface->w/2, 0, surface->w, surface->h};
            SDL_RenderCopy(GameGetData()->renderer, texture, NULL, &rect);
            SDL_FreeSurface(surface);   
            SDL_DestroyTexture(texture);

            RenderButton("Start Game", (int)GameGetScreenSize().x/2, 200, currentMenuOption == 0);
            RenderButton("Options", (int)GameGetScreenSize().x/2, 250, currentMenuOption == 1);
            RenderButton("Quit", (int)GameGetScreenSize().x/2, 300, currentMenuOption == 2);
            break;
        }
        case GAME_STATE_OPTIONS:
        {
            char* str = "Options";
            SDL_Surface* surface = TTF_RenderText_Solid(GameGetData()->font, str, (SDL_Color){255, 255, 255, 255});
            SDL_Texture* texture = SDL_CreateTextureFromSurface(GameGetData()->renderer, surface);
            SDL_Rect rect = {GameGetScreenSize().x/2-surface->w/2, 0, surface->w, surface->h};
            SDL_RenderCopy(GameGetData()->renderer, texture, NULL, &rect);
            SDL_FreeSurface(surface);   
            SDL_DestroyTexture(texture);

            RenderButton("Fullscreen", 300, 100, currentMenuOption == 0);
            RenderSlider("Volume", (int)GameGetScreenSize().x/2, 150, (float)config->volume, currentMenuOption == 1);
            RenderSlider("Left Stick Deadzone", (int)GameGetScreenSize().x/2, 200, (float)config->deadzoneLeft, currentMenuOption == 2);
            RenderSlider("Right Stick Deadzone", (int)GameGetScreenSize().x/2, 250, (float)config->deadzoneRight, currentMenuOption == 3);
            RenderButton("Back", (int)GameGetScreenSize().x/2, 300, currentMenuOption == 4);
            break;
        }
        case GAME_STATE_CHARACTER_SELECTION:
        {
            char* str = "Character Selection";
            SDL_Surface* surface = TTF_RenderText_Solid(GameGetData()->font, str, (SDL_Color){255, 255, 255, 255});
            SDL_Texture* texture = SDL_CreateTextureFromSurface(GameGetData()->renderer, surface);
            SDL_Rect rect = {GameGetScreenSize().x/2-surface->w/2, 0, surface->w, surface->h};
            SDL_RenderCopy(GameGetData()->renderer, texture, NULL, &rect);
            SDL_FreeSurface(surface);   
            SDL_DestroyTexture(texture);

            for (int i = 0; i < 4; i++)
            {
                Player* p = GameGetPlayer(i);
                int width = GameGetScreenSize().x/2 - 50;
                int height = GameGetScreenSize().y/2 - 40;
                int x =  (i % 2) * width + 50;
                int y =  (i / 2) * height + rect.h + 15;
                //TODO Render player previews
                SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
                SDL_RenderDrawRect(renderer, &(SDL_Rect){x, y, width, height});
                surface = TTF_RenderText_Solid(GameGetData()->font, p->skinName, p->isReady ? (SDL_Color){255, 176, 0, 255} : (SDL_Color){255, 255, 255, 255});
                texture = SDL_CreateTextureFromSurface(GameGetData()->renderer, surface);
                rect = (SDL_Rect){x + width/2 - surface->w/2, y + height - surface->h, surface->w, surface->h};
                SDL_RenderCopy(GameGetData()->renderer, texture, NULL, &rect);
            }
            break;
        }
        case GAME_STATE_PLAYING:
        {
            SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
            CameraRenderTexture(&data->camera, data->background, (Vector2){-320, 180}, (Vector2){640, 360});
            BallAimRender();
            bool ballRendered = false;
            bool playersRendered[MAX_PLAYER_COUNT];
            for (int i = 0; i < MAX_PLAYER_COUNT; i++)
            {
                playersRendered[i] = false;

                Player* p = GameGetPlayer(i);
                if (!p->enabled)
                    continue;
                
                if (!PointLeftOfLine(data->net, p->position))
                {
                    PlayerRender(p);
                    playersRendered[i] = true;
                }
            }


            if (!PointLeftOfLine(data->net, data->ball.position))
            {
                BallRender();
                ballRendered = true;
            }

            CameraRenderTexture(&data->camera, data->netTexture, (Vector2){-17, 145}, (Vector2){34, 320});

            if (!ballRendered)
            {
                BallRender();
            }

            for (int i = 0; i < MAX_PLAYER_COUNT; i++)
            {
                Player* p = GameGetPlayer(i);
                if (!p->enabled)
                    continue;

                if (!playersRendered[i])
                    PlayerRender(p);
            }

            #ifdef DEBUG
            SDL_SetRenderDrawColor(renderer, 0xff, 0x00, 0x00, 0xff);
            CameraRenderLine(&data->camera, data->net, renderer);
            #endif

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
            break;
        }
        
        default:
            break;
    }

    SDL_RenderPresent(renderer);
}