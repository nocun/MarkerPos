#pragma once

#include "Marker.h"
#include "Camera.h"

#include <opencv2/opencv.hpp>

#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/GL.h>


// Renders the marker given its previously created texture
void    render(const Marker& marker, GLuint textureId);

// Reads OpenGL view and returns it as a cv::Mat
cv::Mat getRenderedView(int width, int height);

GLuint  createMarkerTexture(const Marker& marker, int textureSize);

// Creates and loads a perspective matrix
void loadPerspectiveMatrix(const Camera& camera, GLdouble nearPlane, GLdouble farPlane);

// These two functions create a perspective by using glFrustum()
void setPerspective(GLdouble left, GLdouble right,
    GLdouble bottom, GLdouble top,
    GLdouble focalX, GLdouble focalY,
    GLdouble principalX, GLdouble principalY,
    GLdouble nearPlane, GLdouble farPlane);

void setPerspective(GLdouble width, GLdouble height,
    GLdouble focalX, GLdouble focalY,
    GLdouble principalX, GLdouble principalY,
    GLdouble nearPlane, GLdouble farPlane);

