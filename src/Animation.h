#ifndef ANIMATION_H
#define ANIMATION_H

#include <list>
#include <obs-module.h>

#include "Keyframe.h"
#include "types.h"

struct AnimationTarget;

struct Animation
{
    const char *property;
    AnimationPropertyType property_type;
    std::list<Keyframe> keyframes;
    AnimationTarget *target;

    Animation(obs_data_t *data, AnimationTarget* target);

    ~Animation()
    {
        delete property;
    }
    
    Keyframe *createFirstKeyframe(obs_data_t *data);
};

#endif