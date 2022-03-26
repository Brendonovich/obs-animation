#include "Animation.h"
#include "Keyframe.h"

Keyframe::Keyframe(obs_data_t *data, Keyframe *last_keyframe, double start_timestamp, AnimationPropertyType type)
{
    double timestamp = obs_data_get_double(data, "timestamp");
    this->duration = timestamp - start_timestamp;

    switch (type)
    {
    case DoubleProperty:
    {
        this->from_double = last_keyframe->to_double;
        this->to_double = obs_data_get_double(data, "value");
        break;
    }
    case IntProperty:
    {
        this->from_int = last_keyframe->to_int;
        this->to_int = obs_data_get_int(data, "value");
        break;
    }
    }
}

Keyframe::Keyframe(double duration, double from, double to)
    : duration(duration), from_double(from), to_double(to)
{
}

Keyframe::Keyframe(double duration, long long from, long long to) : duration(duration), from_int(from), to_int(to)
{
}

void Keyframe::print()
{
    blog(LOG_INFO, "Keyframe: duration: %f, from: %f, to: %f, running_duration: %f", this->duration, this->from_double, this->to_double, this->running_duration);
}