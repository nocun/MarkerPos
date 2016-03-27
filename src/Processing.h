#pragma once

#include "Camera.h"
#include "Marker.h"


// GLUT, OpenGL initialization
void initializeGL(const Camera& camera, const Marker& marker);

// Cleanup
void finalizeGL();
