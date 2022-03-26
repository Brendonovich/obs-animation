#ifndef OBSSOURCE_H
#define OBSSOURCE_H

#include <obs-module.h>

#include "Animation.h"

struct OBSSource
{
    obs_source_t *val;

    inline operator obs_source_t *() const { return val; }
    OBSSource operator=(obs_source_t *ptr) { val = ptr; }

    bool propertyType(const char *property, AnimationPropertyType *output)
    {
        auto properties = obs_source_properties(val);
        auto obs_property = obs_properties_get(properties, property);
        auto obs_property_type = obs_property_get_type(obs_property);

        switch (obs_property_type)
        {
        case OBS_PROPERTY_FLOAT:
            *output = DoubleProperty;
            return true;
        case OBS_PROPERTY_INT:
            *output = IntProperty;
            return true;
        default:
            return false;
        }
    }

    void getDoubleProperty(const char *property, double *output)
    {
        obs_data_t *settings = obs_source_get_settings(val);
        *output = obs_data_get_double(settings, property);
    }

    void getIntProperty(const char *property, long long *output)
    {
        obs_data_t *settings = obs_source_get_settings(val);
        *output = obs_data_get_int(settings, property);
    }
};

#endif