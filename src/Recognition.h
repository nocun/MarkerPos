#pragma once

#include "Marker.h"
#include "Camera.h"

#include <opencv2/opencv.hpp>
#include <vector>


struct MarkerScore {
    Marker marker;
    double score;
};


// Recognizes markers given their 2D image and camera parameters
std::vector<MarkerScore> recognizeMarkers(const Camera& camera, const cv::Mat& sceneRGB);

