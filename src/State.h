#ifndef STATE_H
#define STATE_H

#include <list>

#include "AnimationTarget.h"

struct State
{
    std::list<AnimationTarget> targets;
    bool callback_registered = false;

    static void setAnimation(obs_data_t *request_data, obs_data_t *response_data, void *priv_data);
    static void tick(void *param, float delta);
};

#endif