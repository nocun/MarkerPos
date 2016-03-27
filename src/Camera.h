#pragma once

#include <opencv2/opencv.hpp>


struct Camera {
    // sensor resolution
    int imageWidth  = 640;
    int imageHeight = 480;

    // principal point (image center)
    double principalX = 320.0;
    double principalY = 240.0;

    // focals in pixel units
    double focalX = 500.0;
    double focalY = 500.0;
};


// Returns a 3x3 camera matrix of intrinsic parameters
cv::Mat getCameraMatrix(const Camera& camera);
