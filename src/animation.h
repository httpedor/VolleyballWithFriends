#ifndef ANIMATION_H
#define ANIMATION_H
#include <SDL.h>
#include "game_math.h"

typedef struct {
    SDL_Texture* texture;
    int frameCount;
    Vector2 frameSize;
    int currentFrame;
    float fps;
} Animation;

typedef struct {
    Animation** animations;
    int animationCount;
    int currentAnimation;
    uint32_t lastFrameTime;
    float speedScale;
    bool flipped;
} Animator;

Animation* AnimationCreate(char* fileName, Vector2 frameSize);
void AnimationDelete(Animation* animation);

Animator* AnimatorCreate();
void AnimatorAddAnimation(Animator* animator, Animation* animation);
void AnimatorRemoveAnimation(Animator* animator, Animation* index);
void AnimatorSetCurrentAnimationIndex(Animator* animator, int index);
void AnimatorSetCurrentAnimation(Animator* animator, Animation* animation);
Animation* AnimatorGetCurrentAnimation(Animator* animator);
Animation* AnimatorGetAnimation(Animator* animator, int index);
void AnimatorDelete(Animator* animator, bool deleteAnimations);

void AnimatorUpdate(Animator* animator, double dt);
void AnimatorRender(Animator* animator, Vector2 position, Vector2 size);
#endif // ANIMATION_H
