#include "AnimationTarget.h"
#include <cmath>

AnimationTarget::AnimationTarget(obs_data_t *data)
{
    long long type_int = obs_data_get_int(data, "type");
    type = static_cast<AnimationTargetType>(type_int);

    switch (type)
    {
    case SceneItemTransform:
    {
        const char *scene_name = obs_data_get_string(data, "sceneName");
        int64_t scene_item_id = obs_data_get_int(data, "sceneItemId");

        obs_source_t *scene_source = obs_get_source_by_name(scene_name);
        obs_scene_t *scene = obs_scene_from_source(scene_source);
        obs_sceneitem_t *item = obs_scene_find_sceneitem_by_id(scene, scene_item_id);
        obs_source_release(scene_source);

        obs_target.sceneitem = item;
        break;
    }
    case SourceSettings:
    {
        const char *source_name = obs_data_get_string(data, "sourceName");

        obs_source_t *source = obs_get_source_by_name(source_name);
        obs_source_release(source);

        obs_target.source = source;
        break;
    }
    case FilterSettings:
    {
        const char *source_name = obs_data_get_string(data, "sourceName");
        const char *filter_name = obs_data_get_string(data, "filterName");

        obs_source_t *source = obs_get_source_by_name(source_name);
        obs_source_t *filter = obs_source_get_filter_by_name(source, filter_name);
        obs_source_release(source);
        obs_source_release(filter);

        obs_target.filter = filter;
        break;
    }
    }
}

void AnimationTarget::tick(float delta_millis)
{
    switch (type)
    {
    case SceneItemTransform:
    {
        auto item = this->obs_target.sceneitem;

        obs_transform_info transform = {};
        obs_sceneitem_get_info(item, &transform);

        for (auto animation = this->animations.begin(); animation != this->animations.end(); ++animation)
        {
            auto property = animation->property;
            auto keyframe = &animation->keyframes.front();
            auto duration = keyframe->duration;

            auto t = ((double)keyframe->running_duration >= duration) ? 1.0f : (double)keyframe->running_duration / duration;
            auto value = (keyframe->to_double - keyframe->from_double) * t + keyframe->from_double;

            if (strcmp(property, POSITION_X) == 0)
                transform.pos.x = value;
            else if (strcmp(property, POSITION_Y) == 0)
                transform.pos.y = value;
            else if (strcmp(property, SCALE_X) == 0)
                transform.scale.x = value;
            else if (strcmp(property, SCALE_Y) == 0)
                transform.scale.y = value;
            else if (strcmp(property, ROTATION) == 0)
                transform.rot = value;

            if (keyframe->running_duration >= duration)
            {
                float running_duration_overlap = keyframe->running_duration - (float)duration;

                animation->keyframes.pop_front();

                if (animation->keyframes.empty())
                    animation = this->animations.erase(animation);
                else
                {
                    keyframe = &animation->keyframes.front();
                    keyframe->running_duration = running_duration_overlap;
                }
            }

            keyframe->running_duration += delta_millis;
        }

        obs_sceneitem_set_info(item, &transform);
        break;
    }
    case SourceSettings:
    case FilterSettings:
    {
        auto source = this->obs_target.source;
        auto data = obs_data_create();

        for (auto animation = this->animations.begin(); animation != this->animations.end(); ++animation)
        {
            auto property = animation->property;
            auto keyframe = &animation->keyframes.front();
            auto duration = keyframe->duration;

            auto t = (double)keyframe->running_duration >= duration ? 1.0f : (double)keyframe->running_duration / duration;

            switch (animation->property_type)
            {
            case DoubleProperty:
                obs_data_set_double(data, property, (keyframe->to_double - keyframe->from_double) * t + keyframe->from_double);
                break;
            case IntProperty:
                obs_data_set_int(data, property, (keyframe->to_int - keyframe->from_int) * t + keyframe->from_int);
                break;
            }

            if (keyframe->running_duration >= duration)
            {
                float running_duration_overlap = keyframe->running_duration - (float)duration;

                animation->keyframes.pop_front();

                if (animation->keyframes.empty())
                    animation = this->animations.erase(animation);
                else
                {
                    keyframe = &animation->keyframes.front();
                    keyframe->running_duration = running_duration_overlap;
                }
            }

            keyframe->running_duration += delta_millis;
        }

        obs_source_update(source, data);
        obs_source_update_properties(source);
        obs_data_release(data);
        break;
    }
    }
}