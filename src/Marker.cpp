
#include "Marker.h"

#include <opencv2/opencv.hpp>
#include <cassert>


// Marker and frame size in meters
const double Marker::MARKER_SIZE = 0.3;
const double Marker::FRAME_WIDTH = 0.02;

// NUM_SQUARES*NUM_SQUARES = total number of fields on a marker
const int Marker::NUM_SQUARES = 3;

static_assert(Marker::NUM_SQUARES >= 2, "Need at least 4 squares to calculate rotation");
static_assert(Marker::NUM_SQUARES <= 6, "Maximum 6*6-4=32 bits for marker id");


Marker::Marker(int markerId, Translation trans, Rotation rot)
    : id(markerId), t(trans), r(rot) {
    assert(markerId >= 0);
    assert(markerId < (1 << (NUM_SQUARES*NUM_SQUARES - 4)));
}


int getFrameSize(int markerSizePx) {
    return int(Marker::FRAME_WIDTH / Marker::MARKER_SIZE * markerSizePx);
}


cv::Rect getSquare(int markerSizePx, int squareId) {
    int tex_frame_size = getFrameSize(markerSizePx);
    int tex_square_size = (markerSizePx - 2 * getFrameSize(markerSizePx)) / Marker::NUM_SQUARES;

    int col = squareId % Marker::NUM_SQUARES;
    int row = squareId / Marker::NUM_SQUARES;

    int x = tex_frame_size + col * tex_square_size;
    int y = tex_frame_size + row * tex_square_size;

    return{ cv::Point{ x, y }, cv::Size{ tex_square_size, tex_square_size } };
}


int getCornerSquareId(MarkerCorner corner) {
    int fields = Marker::NUM_SQUARES;

    switch (corner) {
        case UPPER_LEFT:    return 0;
        case UPPER_RIGHT:   return fields - 1;
        case LOWER_LEFT:    return fields * (fields - 1);
        case LOWER_RIGHT:   return fields * fields - 1;
    }

    assert(false);
    return -1;
}


std::vector<cv::Rect> getFrameElements(int markerSizePx) {
    int frameSizePx = getFrameSize(markerSizePx);

    cv::Rect up      { 0, 0, markerSizePx, frameSizePx },
             down    { 0, markerSizePx - frameSizePx, markerSizePx, frameSizePx },
             left    { 0, frameSizePx, frameSizePx, markerSizePx - 2 * frameSizePx },
             right   { markerSizePx - frameSizePx, frameSizePx, frameSizePx, markerSizePx - 2 * frameSizePx };

    return{ up, down, left, right };
}


cv::Rect getSquare(int markerSizePx, MarkerCorner corner) {
    int squareId = getCornerSquareId(corner);
    return getSquare(markerSizePx, squareId);
}


std::vector<cv::Rect> getIdSquares(int markerSizePx) {
    int allFields = Marker::NUM_SQUARES * Marker::NUM_SQUARES;
    std::vector<cv::Rect> result;

    for (int squareId = 1; squareId < allFields-1; squareId++) {
        if (squareId == getCornerSquareId(UPPER_RIGHT) ||
            squareId == getCornerSquareId(LOWER_LEFT)) continue;

        result.push_back(getSquare(markerSizePx, squareId));
    }

    return result;
}






