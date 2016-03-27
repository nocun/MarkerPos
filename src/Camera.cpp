
#include "Camera.h"


cv::Mat getCameraMatrix(const Camera& c) {
    cv::Mat matrix = (cv::Mat_<double>(3, 3) <<
        c.focalX,   0.0,        c.principalX,
        0.0,        c.focalY,   c.principalY,
        0.0,        0.0,        1.0);

    return matrix;
}
