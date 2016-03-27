#pragma once

#include "Transformation.h"

#include <opencv2/opencv.hpp>
#include <vector>


enum MarkerCorner {
    UPPER_LEFT,
    UPPER_RIGHT,
    LOWER_LEFT,
    LOWER_RIGHT
};


struct Marker {
    static const double MARKER_SIZE;
    static const double FRAME_WIDTH;
    static const int    NUM_SQUARES;

    explicit Marker(int markerId = 0, Translation t = {}, Rotation r = {});

    Translation t;
    Rotation    r;
    int         id;
};


// Get all 4 rects that make up the frame
std::vector<cv::Rect> getFrameElements(int markerSizePx);


// Get a particular corner square
cv::Rect getSquare(int markerSizePx, MarkerCorner corner);


// Get squares that make up the id of a marker
// order: vector index == bit number (0 is LSB)
std::vector<cv::Rect> getIdSquares(int markerSizePx);

