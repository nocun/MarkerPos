
#include "Util.h"

#include <algorithm>


double clampAngle(double angle) {
    angle = std::fmod(angle, 360.0);
    if (angle > 180.0)   return angle - 360.0;
    if (angle <= -180.0) return angle + 360.0;
    return angle;
}


double angleDiff(double a1, double a2) {
    a1 = clampAngle(a1);
    a2 = clampAngle(a2);

    double d1 = std::abs(a2 - a1);
    double d2 = std::abs(360.0 + std::min(a1, a2) - std::max(a1, a2));

    return std::min(d1, d2);
}


double deg(double rad) {
    static const double PI = 3.14159265358979323846;
    return rad * 180.0 / PI;
}


double distance(const Translation& t1, const Translation& t2) {
    double x = t2.x - t1.x;
    double y = t2.y - t1.y;
    double z = t2.z - t1.z;

    return std::sqrt(x*x + y*y + z*z);
}


double maxAngleDiff(const Rotation& r1, const Rotation& r2) {
    double d[] = {
        angleDiff(r1.ox, r2.ox),
        angleDiff(r1.oy, r2.oy),
        angleDiff(r1.oz, r2.oz)
    };

    return *std::max_element(std::begin(d), std::end(d));
}

