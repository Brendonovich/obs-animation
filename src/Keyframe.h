#ifndef KEYFRAME_H
#define KEYFRAME_H

#include "types.h"

struct Keyframe
{
    double from_double;
    double to_double;

    long long from_int;
    long long to_int;

    /**
     * The duration of the keyframe in milliseconds
     */
    double duration;
    /**
     * The current timestamp of the keyframe in milliseconds
     */
    float running_duration = 0;

    Keyframe(obs_data_t *data, Keyframe *last_keyframe, double start_timestamp, AnimationPropertyType type);
    Keyframe(double duration, double from, double to);
    Keyframe(double duration, long long from, long long to);
    
    void print();
};

#endif