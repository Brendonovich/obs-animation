#include "Animation.h"
#include "AnimationTarget.h"

Animation::Animation(obs_data_t *data, AnimationTarget *target) : target(target)
{
    obs_data_array_t *keyframes = obs_data_get_array(data, "keyframes");
    const char *property = obs_data_get_string(data, "property");
    int type = obs_data_get_int(data, "type");

    this->property_type = static_cast<AnimationPropertyType>(type);
    this->property = strdup(property);

    auto keyframes_count = obs_data_array_count(keyframes);
    if (keyframes_count == 0)
        return;

    auto first_item = obs_data_array_item(keyframes, 0);
    auto current_timestamp = obs_data_get_double(first_item, "timestamp");
    auto last_keyframe = this->createFirstKeyframe(first_item);

    for (size_t k = 1; k < keyframes_count; k++)
    {
        last_keyframe = &this->keyframes.emplace_back(obs_data_array_item(keyframes, k), last_keyframe, current_timestamp, this->property_type);
        current_timestamp += last_keyframe->duration;
    }
}

Keyframe *Animation::createFirstKeyframe(obs_data_t *data)
{
    switch (this->property_type)
    {
    case DoubleProperty:
    {
        double from;
        switch (this->target->type)
        {
        case SceneItemTransform:
        {
            this->target->obs_target.sceneitem.getDoubleProperty(this->property, &from);
            break;
        }
        case SourceSettings:
        {
            this->target->obs_target.source.getDoubleProperty(this->property, &from);
            break;
        }
        case FilterSettings:
        {
            this->target->obs_target.filter.getDoubleProperty(this->property, &from);
            break;
        }
        }

        double to = obs_data_get_double(data, "value");

        return &this->keyframes.emplace_back(obs_data_get_double(data, "timestamp"), from, to);
    }
    case IntProperty:
    {
        long long from;
        switch (target->type)
        {
        case SceneItemTransform:
        {
            this->target->obs_target.sceneitem.getIntProperty(this->property, &from);
            break;
        }
        case SourceSettings:
        {
            this->target->obs_target.source.getIntProperty(this->property, &from);
            break;
        }
        case FilterSettings:
        {
            this->target->obs_target.filter.getIntProperty(this->property, &from);
            break;
        }
        }

        long long to = obs_data_get_int(data, "value");

        return &this->keyframes.emplace_back(obs_data_get_double(data, "timestamp"), from, to);
    }
    }

    return &this->keyframes.back();
}