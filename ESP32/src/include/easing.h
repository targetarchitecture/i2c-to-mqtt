#ifndef easing_h
#define easing_h

double BounceEaseOut(const double _change, const double _duration, double time_);
double QuarticEaseInOut(const double _change, const double _duration, double time_);
double BounceEaseInOut(const double _change, const double _duration, double time_);
double BounceEaseIn(const double _change, const double _duration, double time_);
double LinearEaseInOut(const double _change, const double _duration, const double time_);

enum easingCurves
{
  // BounceIn,
  // BounceOut,
  BounceInOut,
  // CircularIn,
  // CircularOut,
  // CircularInOut,
  // CubicIn,
  // CubicOut,
  // CubicInOut,
  // ElasticIn,
  // ElasticOut,
  // ElasticInOut,
  // ExponentialIn,
  // ExponentialOut,
  // ExponentialInOut,
  // LinearIn,
  // LinearOut,
  LinearInOut,
  // QuadraticIn,
  // QuadraticOut,
  QuadraticInOut,
  // QuarticIn,
  // QuarticOut,
  // QuarticInOut,
  // QuinticIn,
  // QuinticOut,
  // QuinticInOut,
  // SineIn,
  // SineOut,
  // SineInOut
};

#endif


