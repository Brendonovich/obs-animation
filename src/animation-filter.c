#include "animation-filter.h"
#include <obs-frontend-api.h>
#include <obs-module.h>
#include <stdio.h>
//#include <util/dstr.h>
#include <util/darray.h>
#include <unistd.h>
#include <sys/time.h>

// Keyframes

void keyframe_list_node_free(keyframe_list_node_t *node) {
    bfree(node->keyframe);
    bfree(node);
}

void keyframe_list_remove_first(keyframe_list_t *list) {
    if (list->head == NULL) return;

    keyframe_list_node_t *tmp = list->head;
    if (list->head == list->tail)
        list->tail = NULL;

    list->head = list->head->next;

    keyframe_list_node_free(tmp);
}

void keyframe_list_free(keyframe_list_t *list) {
    keyframe_list_node_t *head = list->head;

    while (head != NULL) {
        keyframe_list_node_t *tmp = head->next;
        keyframe_list_node_free(head);
        head = tmp;
    }

    bfree(list);
}

void keyframe_list_push_back(keyframe_list_t *list, double_keyframe_t *keyframe) {
    keyframe_list_node_t *node = bzalloc(sizeof(keyframe_list_node_t));
    node->keyframe = keyframe;

    if (!list->head)
        list->head = list->tail = node;
    else {
        list->tail->next = node;
        list->tail = node;
    }
}

void keyframe_list_append_tests(keyframe_list_t *list) {
    double_keyframe_t *keyframe_1 = bzalloc(sizeof(double_keyframe_t));
    keyframe_1->duration = 1000;
    keyframe_1->from = 0.0;
    keyframe_1->to = 1920.0;

    keyframe_list_push_back(list, keyframe_1);

    double_keyframe_t *keyframe_2 = bzalloc(sizeof(double_keyframe_t));
    keyframe_2->duration = 1000;
    keyframe_2->from = 1920.0;
    keyframe_2->to = 0;

    keyframe_list_push_back(list, keyframe_2);
}

// Animation

void animation_target_free(animation_target_t *target) {
    target->refs--;
    if (!target->refs) bfree(target);
}

void animation_free(animation_t *animation) {
    keyframe_list_free(animation->keyframes);
    animation_target_free(animation->target);
    bfree(animation);
}

// Animation Info

filter_info_t *filter_info_create() {
    filter_info_t *info = bzalloc(sizeof(filter_info_t));
    da_init(info->animations);

    return info;
}

// Filter

static const char *animation_filter_get_name(void *type_data) {
    UNUSED_PARAMETER(type_data);
    return obs_module_text("Animation");
}

static void *animation_create(obs_data_t *settings, obs_source_t *filter) {
    filter_info_t *info = filter_info_create();
    info->filter = filter;

    da_push_back(state->infos, &info);

    return info;
}

static obs_properties_t *animation_properties(void *data) {
    obs_properties_t *properties = obs_properties_create();
    obs_properties_add_button(properties, "info", obs_module_text("WebSocketControlInfo"), NULL);
    return properties;
}

static void animation_filter_destroy(void *data) {
    filter_info_t *info = data;

    for (size_t i = 0; i < info->animations.num; i++) {
        animation_free(info->animations.array[0]);
    }
    da_free(info->animations);
    bfree(info);
}

static void animation_tick(void *data, float seconds) {
    filter_info_t *info = data;
    float milliseconds = seconds * 1000.0f;

    for (size_t i = 0; i < info->animations.num; i++) {
        animation_t *animation = info->animations.array[i];
        keyframe_list_t *keyframe_list = animation->keyframes;
        double_keyframe_t *keyframe = keyframe_list->head->keyframe;

        float running_duration = keyframe->running_duration;
        float duration = (float) keyframe->duration;

        float t = running_duration >= duration ? 1.0f : running_duration / duration;

        float value = (float) (keyframe->to - keyframe->from) * t + (float) keyframe->from;
        const char *property = animation->property;

        switch (animation->target->type) {
            case ANIMATION_TARGET_SCENE_ITEM_TRANSFORM: {
                obs_sceneitem_t *target_item = animation->target->value.sceneitem;

                struct obs_transform_info transform;
                obs_sceneitem_get_info(target_item, &transform);

                if (strcmp(property, "positionX") == 0)
                    transform.pos.x = value;
                else if (strcmp(property, "positionY") == 0)
                    transform.pos.y = value;
                else if (strcmp(property, "scaleX") == 0)
                    transform.scale.x = value;
                else if (strcmp(property, "scaleY") == 0)
                    transform.scale.y = value;
                else if (strcmp(property, "rotation") == 0)
                    transform.rot = value;

                obs_sceneitem_set_info(target_item, &transform);

                break;
            }
            case ANIMATION_TARGET_SOURCE_SETTINGS: {
                obs_source_t *source = animation->target->value.source;

                obs_data_t *temp = obs_data_create();

                obs_data_set_double(temp, animation->property, value);
                obs_source_update(source, temp);
                obs_source_update_properties(source);

                obs_data_release(temp);
                break;
            }
            case ANIMATION_TARGET_FILTER_SETTINGS: {
                obs_source_t *filter = animation->target->value.filter;

                obs_data_t *temp = obs_data_create();

                obs_data_set_double(temp, animation->property, value);
                obs_source_update(filter, temp);
                obs_source_update_properties(filter);

                obs_data_release(temp);
                break;
            }
        }

        if (keyframe->running_duration >= keyframe->duration) {
            float running_duration_overlap = keyframe->running_duration - (float) keyframe->duration;

            keyframe_list_remove_first(keyframe_list);

            if (keyframe_list->head == NULL) {
                animation_free(animation);
                da_erase(info->animations, i--);
                continue;
            }

            keyframe = keyframe_list->head->keyframe;
            keyframe->running_duration = running_duration_overlap;
        }

        keyframe->running_duration += milliseconds;
    }

    UNUSED_PARAMETER(seconds);
}

struct obs_source_info animation_filter = {
        .id = ANIMATION_FILTER_ID,
        .type = OBS_SOURCE_TYPE_FILTER,
        .output_flags = OBS_SOURCE_VIDEO,
        .get_name = animation_filter_get_name,
        .create = animation_create,
        .destroy = animation_filter_destroy,
        .get_properties = animation_properties,
        .video_tick = animation_tick,
        // .get_defaults = move_source_defaults,
        // .video_render = move_source_video_render,
        // .update = move_source_update,
        // .load = move_source_load,
        // .activate = move_source_activate,
        // .deactivate = move_source_deactivate,
        // .show = move_source_show,
        // .hide = move_source_hide,
};

// Websocket

void find_animation_filter(obs_source_t *parent,
                           obs_source_t *child, void *param) {
    UNUSED_PARAMETER(parent);

    obs_source_t **filter_out = param;
    const char *type = obs_source_get_id(child);

    if (strcmp(type, ANIMATION_FILTER_ID) == 0)
        *filter_out = child;
}

double get_sceneitem_double_property(obs_sceneitem_t *item, const char *property) {
    struct obs_transform_info info;
    obs_sceneitem_get_info(item, &info);

    if (strcmp(property, "positionX") == 0)
        return info.pos.x;
    else if (strcmp(property, "positionY") == 0)
        return info.pos.y;
    else if (strcmp(property, "scaleX") == 0)
        return info.scale.x;
    else if (strcmp(property, "scaleY") == 0)
        return info.scale.y;
    else if (strcmp(property, "rotation") == 0)
        return info.rot;
    else return 0;
}

void set_animation_cb(obs_data_t *request_data, obs_data_t *response_data, void *priv_data) {
    UNUSED_PARAMETER(response_data);
    UNUSED_PARAMETER(priv_data);

    obs_data_array_t *targets = obs_data_get_array(request_data, "targets");
    size_t targets_count = obs_data_array_count(targets);
    obs_data_t *animations = obs_data_get_obj(request_data, "animations");

    for (size_t i = 0; i < targets_count; i++) {
        obs_data_t *target_data = obs_data_array_item(targets, i);

        const char *ref = obs_data_get_string(target_data, "ref");
        animation_target_type_t target_type = obs_data_get_int(target_data, "type");

        obs_data_array_t *target_animations = obs_data_get_array(animations, ref);
        size_t target_animations_count = obs_data_array_count(target_animations);

        animation_target_t *target = bzalloc(sizeof(animation_target_t));
        target->type = target_type;
        target->refs = target_animations_count;

        obs_source_t *filter_parent = NULL;

        switch (target_type) {
            case ANIMATION_TARGET_SCENE_ITEM_TRANSFORM: {
                const char *scene_name = obs_data_get_string(target_data, "sceneName");
                int64_t scene_item_id = obs_data_get_int(target_data, "sceneItemId");

                obs_source_t *scene_source = obs_get_source_by_name(scene_name);
                obs_scene_t *scene = obs_scene_from_source(scene_source);
                obs_sceneitem_t *item = obs_scene_find_sceneitem_by_id(scene, scene_item_id);

                target->value.sceneitem = item;
                filter_parent = obs_scene_get_source(scene);

                obs_source_release(scene_source);

                break;
            }
            case ANIMATION_TARGET_SOURCE_SETTINGS: {
                const char *source_name = obs_data_get_string(target_data, "sourceName");

                obs_source_t *source = obs_get_source_by_name(source_name);
                obs_source_release(source);

                target->value.source = source;
                filter_parent = source;

                break;
            }
            case ANIMATION_TARGET_FILTER_SETTINGS: {
                const char *source_name = obs_data_get_string(target_data, "sourceName");
                const char *filter_name = obs_data_get_string(target_data, "filterName");

                obs_source_t *source = obs_get_source_by_name(source_name);
                obs_source_release(source);
                obs_source_t *filter = obs_source_get_filter_by_name(source, filter_name);
                obs_source_release(filter);

                target->value.filter = filter;
                filter_parent = source;

                break;
            }
        }

        obs_source_t *target_filter = NULL;
        obs_source_enum_filters(filter_parent, find_animation_filter, &target_filter);
        if (!target_filter) continue;

        filter_info_t *target_filter_info = NULL;

        for (size_t j = 0; j < state->infos.num; j++) {
            filter_info_t *tmp = state->infos.array[j];

            if (tmp->filter == target_filter) target_filter_info = tmp;
        }

        if (!target_filter_info) continue;

        for (size_t j = 0; j < target_animations_count; j++) {
            obs_data_t *target_animation = obs_data_array_item(target_animations, j);

            obs_data_array_t *animation_keyframes = obs_data_get_array(target_animation, "keyframes");
            size_t keyframes_count = obs_data_array_count(animation_keyframes);
            if (keyframes_count == 0) continue;

            const char *property = obs_data_get_string(target_animation, "property");

            obs_data_t *first_item = obs_data_array_item(animation_keyframes, 0);
            double_keyframe_t *last_keyframe = bzalloc(sizeof(double_keyframe_t));

            last_keyframe->to = obs_data_get_double(first_item, "value");
            last_keyframe->duration = obs_data_get_double(first_item, "timestamp");

            switch (target_type) {
                case ANIMATION_TARGET_SCENE_ITEM_TRANSFORM: {
                    last_keyframe->from = get_sceneitem_double_property(target->value.sceneitem, property);
                    break;
                }
                case ANIMATION_TARGET_SOURCE_SETTINGS: {
                    obs_data_t *settings = obs_source_get_settings(target->value.source);
                    last_keyframe->from = obs_data_get_double(settings, property);
                    break;
                }
                case ANIMATION_TARGET_FILTER_SETTINGS: {
                    obs_data_t *settings = obs_source_get_settings(target->value.filter);
                    last_keyframe->from = obs_data_get_double(settings, property);
                    break;
                }
            }

            keyframe_list_t *keyframes = bzalloc(sizeof(keyframe_list_t));
            keyframe_list_push_back(keyframes, last_keyframe);

            double current_timestamp = last_keyframe->duration;

            for (size_t k = 1; k < keyframes_count; k++) {
                obs_data_t *keyframe_data = obs_data_array_item(animation_keyframes, k);

                double value = obs_data_get_double(keyframe_data, "value");
                double timestamp = obs_data_get_double(keyframe_data, "timestamp");

                double_keyframe_t *keyframe = bzalloc(sizeof(double_keyframe_t));
                keyframe->from = last_keyframe->to;
                keyframe->duration = timestamp - current_timestamp;
                keyframe->to = value;

                keyframe_list_push_back(keyframes, keyframe);

                last_keyframe = keyframe;
                current_timestamp += keyframe->duration;
            }

            animation_t *animation = bzalloc(sizeof(animation_t));
            animation->keyframes = keyframes;
            animation->target = target;
            animation->property = strdup(property);

            da_push_back(target_filter_info->animations, &animation);
        }
    }
}