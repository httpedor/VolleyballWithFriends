#define SDL_MAIN_HANDLED 
#include <SDL.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include "game.h"


int main(int argc, char* argv[]) {
    printf("Iniciando SDL\n");
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0)
    {
        fprintf(stderr, "Não foi possível inicializar o SDL: %s\n", SDL_GetError());
        return 1;
    }

    printf("Iniciando TTF\n");
    if (TTF_Init() != 0)
    {
        fprintf(stderr, "Não foi possível inicializar o SDL_ttf: %s\n", TTF_GetError());
        return 1;
    }

    printf("Iniciando Jogo\n");
    if (GameInit() == NULL)
    {
        fprintf(stderr, "Não foi possível inicializar o jogo\n");
        return 1;
    }

    uint64_t NOW = SDL_GetPerformanceCounter();
    uint64_t LAST = NOW;
    double ellapsedTime = 0;
    double fpsUpdateTime = 0;
    int frame = 0;

    double deltaTime = 0;
    double fps = 0;
    printf("Iniciando Loop Principal\n");
    while (GameGetData()->shouldQuit == 0)
    {
        NOW = SDL_GetPerformanceCounter();
        deltaTime = (double)((NOW - LAST) / (double)SDL_GetPerformanceFrequency());
        LAST = NOW;
        ellapsedTime += deltaTime;
        fpsUpdateTime += deltaTime;

        if (fpsUpdateTime>= 1)
        {
            fps = (double)(frame) / fpsUpdateTime;
            frame = 0;
            fpsUpdateTime = 0;
        }

        GameInput();
        if (!GameGetData()->paused)
        {
            GameProcess(deltaTime * GameGetTimeScale());
        }
        GameRender(deltaTime, fps);

        frame++;
        if (deltaTime < 1.0 / (TICK_RATE*2))
            SDL_Delay(1);
    }

    GameQuit();
    return 0;
}