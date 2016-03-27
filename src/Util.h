#pragma once

#include "Transformation.h"


// Keep angles in the (-180.0; 180] range
double  clampAngle(double angle);

// Convert radians to degrees
double  deg(double rad);

// Calculate the Euclidean distance between 2 translations
double  distance(const Translation& t1, const Translation& t2);

// Calculate maximum angle difference between ox, oy and oz pairs
double  maxAngleDiff(const Rotation& r1, const Rotation& r2);
