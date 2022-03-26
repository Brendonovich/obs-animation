#include "State.h"
#include <thread>
#include <mutex>

State state;

void State::tick(void *param, float delta)
{
    UNUSED_PARAMETER(param);

    float delta_millis = delta * 1000.0f;

    for (auto target = state.targets.begin(); target != state.targets.end(); ++target)
    {
        target->tick(delta_millis);

        if (target->animations.empty())
        {
            target = state.targets.erase(target);
        }
    }
}

void State::setAnimation(obs_data_t *request_data, obs_data_t *response_data, void *priv_data)
{
    UNUSED_PARAMETER(response_data);
    UNUSED_PARAMETER(priv_data);

    auto targets_array = obs_data_get_array(request_data, "targets");

    for (size_t i = 0; i < obs_data_array_count(targets_array); i++)
    {
        auto target_data = obs_data_array_item(targets_array, i);
        auto &target = state.targets.emplace_back(target_data);
        auto animations = obs_data_get_array(target_data, "animations");

        for (size_t j = 0; j < obs_data_array_count(animations); j++)
            target.animations.emplace_back(obs_data_array_item(animations, j), &target);
    }
}