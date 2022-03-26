#ifndef ANIMATIONTARGET_H
#define ANIMATIONTARGET_H

#include <list>
#include <obs-module.h>

#include "Animation.h"
#include "OBSSceneItem.h"
#include "OBSSource.h"
#include "types.h"

struct AnimationTarget
{
    AnimationTargetType type;
    union AnimationTargetUnion
    {
        OBSSceneItem sceneitem; // SceneItemTransform
        OBSSource source;       // SourceSettings
        OBSSource filter;       // FilterSettings
    } obs_target;
    std::list<Animation> animations;

    AnimationTarget(obs_data_t *data);
    
    void tick(float delta_millis);
};

#endif