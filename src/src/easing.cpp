#include <Arduino.h>
#include "easing.h"

double QuarticEaseInOut(const double _change, const double _duration, double time_)
{
    time_ /= _duration / 2;

    if (time_ < 1)
        return _change / 2 * time_ * time_ * time_ * time_;

    time_ -= 2;
    return -_change / 2 * (time_ * time_ * time_ * time_ - 2);
}

double BounceEaseIn(const double _change, const double _duration, const double time_)
{
    return _change - BounceEaseOut(_change, _duration, _duration - time_);
}

double BounceEaseOut(const double _change, const double _duration, double time_)
{
    time_ /= _duration;

    if (time_ < (1 / 2.75))
        return _change * (7.5625 * time_ * time_);

    if (time_ < (2 / 2.75))
    {
        time_ -= 1.5 / 2.75;
        return _change * (7.5625 * time_ * time_ + 0.75);
    }

    if (time_ < (2.5 / 2.75))
    {
        time_ -= 2.25 / 2.75;
        return _change * (7.5625 * time_ * time_ + 0.9375);
    }

    time_ -= 2.625 / 2.75;
    return _change * (7.5625 * time_ * time_ + 0.984375);
}

double BounceEaseInOut(const double _change, const double _duration, const double time_)
{
    if (time_ < _duration / 2)
        return BounceEaseIn(_change, _duration, time_ * 2) * 0.5;
    else
        return BounceEaseOut(_change, _duration, time_ * 2 - _duration) * 0.5 + _change * 0.5;
}

double LinearEaseInOut(const double _change, const double _duration, const double time_)
{
    return _change * time_ / _duration;
}
