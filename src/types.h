#ifndef TYPES_H
#define TYPES_H

typedef enum ColourInterpolateMode
{
    COLOUR_INTERPOLATE_MODE_RGB,
    COLOUR_INTERPOLATE_MODE_HSL
} ColourInterpolateMode;

enum AnimationPropertyType
{
    DoubleProperty,
    IntProperty
};

enum AnimationTargetType
{
    SceneItemTransform,
    SourceSettings,
    FilterSettings
};

#endif