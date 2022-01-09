#include <obs-module.h>
#include <util/darray.h>
#include "../vendor/obs-websocket-api.h"

#define ANIMATION_FILTER_ID "animation_filter"

typedef enum {
    ANIMATION_TYPE_DOUBLE
} animation_type;

typedef enum animation_target_type {
    ANIMATION_TARGET_SCENE_ITEM_TRANSFORM,
    ANIMATION_TARGET_SOURCE_SETTINGS,
    ANIMATION_TARGET_FILTER_SETTINGS
} animation_target_type_t;

typedef struct animation_target {
    animation_target_type_t type;
    size_t refs;

    union animation_target_union {
        obs_sceneitem_t *sceneitem; // ANIMATION_TARGET_SCENE_ITEM_TRANSFORM
        obs_source_t *source; // ANIMATION_TARGET_SOURCE_SETTINGS
        obs_source_t *filter; // ANIMATION_TARGET_FILTER_SETTINGS
    } value;
} animation_target_t;

typedef struct double_keyframe {
    double from;
    double to;

    /**
     * The duration of the keyframe in milliseconds
     */
    double duration;
    /**
     * The current timestamp of the keyframe in milliseconds
     */
    float running_duration;
} double_keyframe_t;

typedef struct keyframe_list_node {
    double_keyframe_t *keyframe;
    struct keyframe_list_node *next;
} keyframe_list_node_t;

typedef struct keyframe_list {
    keyframe_list_node_t *head;
    keyframe_list_node_t *tail;
} keyframe_list_t;

typedef struct animation {
    animation_target_t *target;
    const char *property;
    keyframe_list_t *keyframes;
} animation_t;

typedef struct filter_info {
    obs_source_t *filter;
    DARRAY(animation_t *) animations;
} filter_info_t;

typedef struct global_state {
    DARRAY(filter_info_t *) infos;
} global_state_t;

global_state_t *state;

void set_animation_cb(obs_data_t *, obs_data_t *, void *);
