#include "OBSSceneItem.h"

bool OBSSceneItem::getDoubleProperty(const char *property, double *output)
{
    double value;

    obs_transform_info info = {};
    obs_sceneitem_get_info(val, &info);

    if (strcmp(property, POSITION_X) == 0)
        value = info.pos.x;
    else if (strcmp(property, POSITION_Y) == 0)
        value = info.pos.y;
    else if (strcmp(property, SCALE_X) == 0)
        value = info.scale.x;
    else if (strcmp(property, SCALE_Y) == 0)
        value = info.scale.y;
    else if (strcmp(property, BOUNDS_X) == 0)
        value = info.bounds.x;
    else if (strcmp(property, BOUNDS_Y) == 0)
        value = info.bounds.y;
    else if (strcmp(property, ROTATION) == 0)
        value = info.rot;
    else
        return false;

    *output = value;
    return true;
}

bool OBSSceneItem::getIntProperty(const char *property, long long *output)
{
    obs_transform_info info = {};
    obs_sceneitem_get_info(val, &info);

    if (strcmp(property, ALIGNMENT) == 0)
        *output = info.alignment;
    else if (strcmp(property, BOUNDS_TYPE) == 0)
        *output = info.bounds_type;
    else if (strcmp(property, BOUNDS_ALIGNMENT) == 0)
        *output = info.bounds_alignment;
    else
        return false;

    return true;
}

bool OBSSceneItem::propertyType(const char *property, AnimationPropertyType *output)
{
    if (strcmp(property, POSITION_X) == 0 || strcmp(property, POSITION_Y) == 0 ||
        strcmp(property, SCALE_X) == 0 || strcmp(property, SCALE_Y) == 0 ||
        strcmp(property, ROTATION) == 0)
    {
        *output = DoubleProperty;
        return true;
    }
    else if (strcmp(property, ALIGNMENT) == 0 || strcmp(property, BOUNDS_TYPE) == 0 ||
             strcmp(property, BOUNDS_ALIGNMENT) == 0)
    {
        *output = IntProperty;
        return true;
    }

    return false;
}