#include "animation.h"
#include "game.h"
#include <stdio.h>

Animation* AnimationCreate(char* fileName, Vector2 frameSize)
{
    Animation* animation = malloc(sizeof(Animation));
    SDL_Surface* surface = SDL_LoadBMP(fileName);
    if (surface == NULL)
    {
        free(animation);
        printf("Error loading image %s: %s\n", fileName, SDL_GetError());
        return NULL;
    }
    animation->texture = SDL_CreateTextureFromSurface(GameGetData()->renderer, surface);
    SDL_FreeSurface(surface);
    SDL_Point iframeSize;
    SDL_QueryTexture(animation->texture, NULL, NULL, &iframeSize.x, &iframeSize.y);
    animation->frameSize = frameSize;
    animation->frameCount = (iframeSize.x / frameSize.x) * (iframeSize.y / frameSize.y);
    animation->currentFrame = 0;
    animation->fps = 24;
    animation->firstFrame = 0;
    animation->lastFrame = animation->frameCount - 1;
    animation->loop = true;
    return animation;
}
void AnimationDelete(Animation* animation)
{
    SDL_DestroyTexture(animation->texture);
    free(animation);
}

Animator* AnimatorCreate()
{
    Animator* animator = malloc(sizeof(Animator));
    animator->animations = NULL;
    animator->animationCount = 0;
    animator->currentAnimation = 0;
    animator->lastFrameTime = 0;
    animator->speedScale = 1;
    return animator;
}
void AnimatorAddAnimation(Animator* animator, Animation* animation)
{
    animator->animations = realloc(animator->animations, sizeof(Animation*) * (animator->animationCount + 1));
    animator->animations[animator->animationCount] = animation;
    animator->animationCount++;
}
void AnimatorRemoveAnimation(Animator* animator, Animation* animation)
{
    if (animation == NULL)
        return;
    for (int i = 0; i < animator->animationCount; i++)
    {
        Animation* anim = animator->animations[i];
        if (anim == animation)
        {
            for (int j = i; j < animator->animationCount - 1; j++)
            {
                animator->animations[j] = animator->animations[j + 1];
            }
            animator->animationCount--;
            animator->animations = realloc(animator->animations, sizeof(Animation*) * animator->animationCount);
            return;
        }
    }
}
void AnimatorSetCurrentAnimationIndex(Animator* animator, int index)
{
    int oldIndex = animator->currentAnimation;
    if (oldIndex != index)
    {
        animator->currentAnimation = index;
        AnimatorGetCurrentAnimation(animator)->currentFrame = AnimatorGetCurrentAnimation(animator)->firstFrame;
    }
}
void AnimatorSetCurrentAnimation(Animator* animator, Animation* animation)
{
    for (int i = 0; i < animator->animationCount; i++)
    {
        if (animator->animations[i] == animation)
        {
            AnimatorSetCurrentAnimationIndex(animator, i);
            return;
        }
    }
}
Animation* AnimatorGetCurrentAnimation(Animator* animator)
{
    return animator->animations[animator->currentAnimation];
}
Animation* AnimatorGetAnimation(Animator* animator, int index)
{
    return animator->animations[index];
}
void AnimatorDelete(Animator* animator, bool deleteAnimations)
{
    if (deleteAnimations)
    {
        for (int i = 0; i < animator->animationCount; i++)
        {
            AnimationDelete(animator->animations[i]);
        }
    }
    free(animator->animations);
    free(animator);
}

void AnimatorUpdate(Animator* animator, double dt)
{
    if (animator->animationCount == 0 || animator->animations[animator->currentAnimation] == NULL || animator->speedScale == 0)
        return;

    Animation* current = animator->animations[animator->currentAnimation];
    if (current->fps == 0)
        return;

    uint32_t currentTime = SDL_GetTicks();
    uint32_t deficit = currentTime - animator->lastFrameTime;
    while(deficit >= 1000 / (current->fps * animator->speedScale))
    {
        animator->lastFrameTime = currentTime;
        current->currentFrame++;
        if (current->currentFrame > current->lastFrame)
        {
            if (current->loop)
                current->currentFrame = current->firstFrame;
            else
                current->currentFrame = current->lastFrame;
        }
        deficit -= 1000 / (current->fps * animator->speedScale);
    }
}
void AnimatorRender(Animator* animator, Vector2 position, Vector2 size)
{
    if (animator->animationCount == 0)
        return;

    Animation* current = animator->animations[animator->currentAnimation];
    SDL_Rect src = {current->frameSize.x * current->currentFrame, 0, current->frameSize.x, current->frameSize.y};
    SDL_FRect dst = {position.x, position.y, size.x, size.y};
    SDL_RenderCopyExF(GameGetData()->renderer, current->texture, &src, &dst, 0, NULL, animator->flipped ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
}