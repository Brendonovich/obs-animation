#ifndef OBSSCENEITEM_H
#define OBSSCENEITEM_H

#include <obs-module.h>

#include "types.h"

#define POSITION_X "positionX"
#define POSITION_Y "positionY"
#define SCALE_X "scaleX"
#define SCALE_Y "scaleY"
#define BOUNDS_X "boundsX"
#define BOUNDS_Y "boundsY"
#define ROTATION "rotation"
#define ALIGNMENT "alignment"
#define BOUNDS_TYPE "boundsType"
#define BOUNDS_ALIGNMENT "boundsAlignment"

struct OBSSceneItem
{
    obs_sceneitem_t *val;

    inline operator obs_sceneitem_t *() const { return val; }
    OBSSceneItem operator=(obs_sceneitem_t *ptr) { val = ptr; }

    bool getDoubleProperty(const char *property, double *output);
    bool getIntProperty(const char *property, long long *output);

    static bool propertyType(const char *property, AnimationPropertyType *output);
};

#endif