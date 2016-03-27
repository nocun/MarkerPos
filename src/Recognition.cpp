
#include "Recognition.h"
#include "Marker.h"
#include "Camera.h"
#include "Util.h"

#include <opencv2/opencv.hpp>
#include <algorithm>
#include <numeric>
#include <cassert>
#include <iomanip>


const int       NORMALIZED_MARKER_SIZE  = 256;      // pixels (square side size)
const int       MIN_CONTOUR_LEN         = 64;       // pixels (circumference)
const double    MIN_QUAD_AREA           = 64.0;     // pixels^2
const double    VALID_MARKER_TOLERANCE  = 0.2;      // percentage (0.0 - 1.0)
const double    BINARIZATION_THRESHOLD  = 127.0;    // grey value 0.0 - 255.0


typedef std::vector<cv::Point> Contour;
typedef std::vector<cv::Point2f> ContourFloat;


// Rotate image by n*90 degrees;
void rotate90(cv::Mat& img, int angle) {
    assert(angle % 90 == 0);
    angle = ((angle % 360) + 360) % 360;

    if (angle == 90) {
        cv::flip(img.t(), img, 1);
    }
    else if (angle == 180) {
        cv::flip(img, img, -1);
    }
    else if (angle == 270) {
        cv::flip(img.t(), img, 0);
    }
}


// Rotate given transformation by 180 degrees around OX
cv::Mat rotateOx180(const cv::Mat& transformationMatrix) {
    assert(transformationMatrix.cols == 4 && transformationMatrix.rows == 4);

    cv::Mat rot180 = (cv::Mat_<double>(4, 4) <<
        1.0,  0.0,  0.0, 0.0,
        0.0, -1.0,  0.0, 0.0,
        0.0,  0.0, -1.0, 0.0,
        0.0,  0.0,  0.0, 1.0);

    return rot180 * transformationMatrix;
}


// Extract contours after grey image binarization
std::vector<Contour> findContours(const cv::Mat& sceneGrey, int minContourLen = 0) {
    std::vector<Contour> contours;
    cv::Mat sceneBinary;

    cv::threshold(sceneGrey, sceneBinary, BINARIZATION_THRESHOLD, 0.0, CV_THRESH_TOZERO);
    cv::findContours(sceneBinary, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
    
    contours.erase(
        std::remove_if(std::begin(contours), std::end(contours),
            [&minContourLen](const std::vector<cv::Point>& e) {
                return int(e.size()) < minContourLen;
            }),
        std::end(contours));

    return contours;
}


ContourFloat convertToFloat(const Contour& contour) {
    ContourFloat result(contour.size());

    for (std::size_t i = 0; i < result.size(); i++) {
        result[i] = { float(contour[i].x), float(contour[i].y) };
    }

    return result;
}


// Make sure the 4 vertices are ordered properly
void setClockwiseOrder(Contour& quad) {
    assert(quad.size() == 4);

    cv::Point vAB = quad[1] - quad[0];
    cv::Point vAC = quad[2] - quad[0];

    int cross = (vAB.x * vAC.y) - (vAB.y * vAC.x);

    if (cross < 0) {
        std::swap(quad[1], quad[3]);
    }
}


// Convert contours to quadrangles with precise corners
std::vector<ContourFloat> getPreciseQuads(const cv::Mat& sceneGrey, const std::vector<Contour>& contours, double minQuadArea = 0.0) {
    std::vector<ContourFloat> result;
    ContourFloat preciseQuad;
    Contour quad;

    cv::TermCriteria termCriteria { cv::TermCriteria::MAX_ITER | cv::TermCriteria::EPS, 30, 0.01 };

    for (auto& contour : contours) {
        double eps = contour.size() * 0.05;

        cv::approxPolyDP(contour, quad, eps, true);

        if (quad.size() == 4 &&
            cv::isContourConvex(quad) &&
            cv::contourArea(quad) >= minQuadArea) {
            
            setClockwiseOrder(quad);
            preciseQuad = convertToFloat(quad);

            cv::cornerSubPix(sceneGrey, preciseQuad, cv::Size{ 3, 3 }, cv::Size{ -1, -1 }, termCriteria);

            result.push_back(preciseQuad);
        }
    }
    
    return result;
}


cv::Mat get2DPerspectiveTransform(const ContourFloat& quad, int normalizedMarkerSize) {
    float size = float(normalizedMarkerSize - 1);

    ContourFloat normalizedQuad = {
        { 0.0f, 0.0f },
        { size, 0.0f },
        { size, size },
        { 0.0f, size } };

    return cv::getPerspectiveTransform(quad, normalizedQuad);
}


// Calculate undistorted marker rotation (one of -90, 0, 90, 180)
int getMarkerRotation(const cv::Mat& markerImg) {
    std::vector<MarkerCorner> corners = { UPPER_LEFT, UPPER_RIGHT, LOWER_LEFT, LOWER_RIGHT };
    std::vector<int> rotations = { 90, 0, 180, -90 };
    double minValue = std::numeric_limits<double>::infinity();

    std::size_t index = 0;

    for (std::size_t i = 0; i < 4; i++) {
        auto square = getSquare(markerImg.cols, corners[i]);
        double color = cv::mean(markerImg(square))[0];

        if (color < minValue) {
            index = i;
            minValue = color;
        }
    }

    return rotations[index];
}


// Check if marker image has a proper white frame along the edges
bool hasWhiteFrame(const cv::Mat& markerImg, double tolerance) {
    auto frame = getFrameElements(markerImg.cols);
    double value = 0.0;

    for (const auto& rect : frame) {
        value += cv::mean(markerImg(rect))[0];
    }

    return (value / frame.size()) > ((1.0-tolerance) * 255.0);
}


// Check if the 4 corner squares are correct: 3 white and 1 black
bool hasValidCorners(const cv::Mat& markerImg, double tolerance) {
    std::vector<MarkerCorner> whiteCorners = { UPPER_LEFT, LOWER_LEFT, LOWER_RIGHT };
    double highValue = (1.0 - tolerance) * 255.0;
    double lowValue = tolerance * 255.0;

    // first check the black corner
    auto square = getSquare(markerImg.cols, UPPER_RIGHT);
    double color = cv::mean(markerImg(square))[0];

    if (color > lowValue)
        return false;

    for (auto corner : whiteCorners) {
        square = getSquare(markerImg.cols, corner);
        color = cv::mean(markerImg(square))[0];

        if (color < highValue)
            return false;
    }

    return true;
}


// Check if the marker is correct - must have a white frame, 3 white and 1 black corners
bool isMarkerValid(const cv::Mat& markerImg, double tolerance) {
    return hasWhiteFrame(markerImg, tolerance) &&
           hasValidCorners(markerImg, tolerance);
}


// Warp marker images from arbitrary quads into squares
std::vector<cv::Mat> undistortMarkerImages(const cv::Mat& sceneGrey, const std::vector<ContourFloat>& quads, int normalizedMarkerSize) {
    cv::Size markerSize{ normalizedMarkerSize, normalizedMarkerSize };
    std::vector<cv::Mat> result(quads.size());
    
    for (std::size_t i = 0; i < result.size(); i++) {
        auto perspective = get2DPerspectiveTransform(quads[i], normalizedMarkerSize);
        cv::warpPerspective(sceneGrey, result[i], perspective, markerSize, cv::INTER_NEAREST);
    }

    return result;
}


// Rotate markers into upright position, rotate quad verticies accordingly
void rotateMarkers(std::vector<cv::Mat>& markerImgs, std::vector<ContourFloat>& quads) {
    assert(markerImgs.size() == quads.size());

    for (std::size_t i = 0; i < markerImgs.size(); i++) {

        int rotation = getMarkerRotation(markerImgs[i]);

        rotate90(markerImgs[i], rotation);

        int rotPointsBy = (4 - rotation / 90) % 4;
        std::rotate(std::begin(quads[i]), std::begin(quads[i]) + rotPointsBy, std::end(quads[i]));
    }
}


// Remove invalid markers along with their corresponding quadrangles
void filterInvalidMarkers(std::vector<cv::Mat>& markerImgs, std::vector<ContourFloat>& quads, double tolerance) {
    assert(markerImgs.size() == quads.size());

    for (int i = int(markerImgs.size())-1; i >= 0; i--) {
        if (!isMarkerValid(markerImgs[i], tolerance)) {
            markerImgs.erase(std::begin(markerImgs) + i);
            quads.erase(std::begin(quads) + i);
        }
    }
}


// Calculate marker ID
int getId(const cv::Mat& markerImg) {
    int bitIndex = 0;
    int id = 0;
    
    for (const auto& square : getIdSquares(markerImg.cols)) {
        int bitValue = cv::mean(markerImg(square))[0] < 127.0 ? 0 : 1;
        id |= bitValue << bitIndex++;
    }

    return id;
}


// Calculate marker recognition score given its ID
double calculateScore(const cv::Mat& markerImg, int id) {
    double totalSum = 0.0;

    // all frame elements
    for (const auto& rect : getFrameElements(markerImg.cols)) {
        totalSum += cv::sum(markerImg(rect))[0];
    }

    // 3 white corners
    for (const auto& corner : { UPPER_LEFT, LOWER_LEFT, LOWER_RIGHT }) {
        auto square = getSquare(markerImg.cols, corner);
        totalSum += cv::sum(markerImg(square))[0];
    }

    // 1 black corner
    auto square = getSquare(markerImg.cols, UPPER_RIGHT);
    totalSum += 255.0 * square.area() - cv::sum(markerImg(square))[0];

    // marker id squares
    auto idSqaures = getIdSquares(markerImg.cols);

    for (int bitIndex = 0; bitIndex < int(idSqaures.size()); bitIndex++) {

        double sum = cv::sum(markerImg(idSqaures[bitIndex]))[0];

        // the square could be white
        if (id & (1 << bitIndex)) {
            totalSum += sum;
        }

        // or black
        else {
            totalSum += 255.0 * idSqaures[bitIndex].area() - sum;
        }
    }

    return totalSum / markerImg.size().area() / 255.0;
}


// Convert rotation matrix into Euler angles
Rotation getEulerAngles(const cv::Mat& r) {
    double m00 = r.at<double>(0, 0);
    double m01 = r.at<double>(0, 1);
    double m02 = r.at<double>(0, 2);
    double m10 = r.at<double>(1, 0);
    double m11 = r.at<double>(1, 1);
    double m12 = r.at<double>(1, 2);
    double m20 = r.at<double>(2, 0);
    double m21 = r.at<double>(2, 1);
    double m22 = r.at<double>(2, 2);

    double ox = std::atan2(m12, m22);
    double c2 = std::sqrt(m00*m00 + m01*m01);
    double oy = std::atan2(-m02, c2);
    double s1 = std::sin(ox);
    double c1 = std::cos(ox);
    double oz = std::atan2(s1*m20 - c1*m10, c1*m11 - s1*m21);

    return{ deg(ox), deg(oy), deg(oz) };
}


// Extract translation vector
Translation getTranslation(const cv::Mat& r) {
    double x = r.at<double>(0, 3);
    double y = r.at<double>(1, 3);
    double z = r.at<double>(2, 3);

    return{ x, y, z };
}


// Calculate 3D transformation between 2D image points and their 3D counterparts
Transformation calculateTransformation(const Camera& camera,
    const std::vector<cv::Point2f>& points2D,
    const std::vector<cv::Point3f>& points3D) {

    assert(points2D.size() == points3D.size());

    cv::Mat cameraMatrix = getCameraMatrix(camera);
    cv::Mat distortionCoefficients = {};

    cv::Mat transformationMatrix44{ 4, 4, CV_64FC1, 1.0 };
    cv::Mat rotationMatrix33, translationVec, rotationVec;

    cv::solvePnP(points3D, points2D, cameraMatrix, distortionCoefficients, rotationVec, translationVec, false);
    cv::Rodrigues(rotationVec, rotationMatrix33);

    // create a 4x4 transformation matrix
    rotationMatrix33.copyTo(transformationMatrix44({ 0, 0, 3, 3 }));
    translationVec.copyTo(transformationMatrix44({ 3, 0, 1, 3 }));

    // convert from OpenCV camera space to OpenGL camera space
    // by rotating 180 degrees around OX
    auto openGLMatrix = rotateOx180(transformationMatrix44);

    Translation t = getTranslation(openGLMatrix);
    Rotation r = getEulerAngles(openGLMatrix);
    
    // remove our OX rotation information
    r.ox = clampAngle(180.0 - r.ox);
    r.oy = clampAngle(r.oy);
    r.oz = clampAngle(r.oz);

    return{ t, r };
}


// Calculate quadrangle 3D transformation
Transformation calculateTransformation(const Camera& camera, const ContourFloat& quad) {
    auto half = float(Marker::MARKER_SIZE) / 2.0f;

    std::vector<cv::Point3f> markerCorners3D = {
        { -half, -half, 0.0f },
        { +half, -half, 0.0f },
        { +half, +half, 0.0f },
        { -half, +half, 0.0f } };

    return calculateTransformation(camera, quad, markerCorners3D);
}


// Show debug information
void debugMarkers(const cv::Mat& sceneRGB,
                  const std::vector<cv::Mat>& markerImgs,
                  const std::vector<MarkerScore>& markers,
                  const std::vector<ContourFloat>& quads) {

    assert(markerImgs.size() == markers.size());
    assert(markerImgs.size() == quads.size());

    // helper function
    auto putText = [](cv::Mat& img, const std::string& str, cv::Point pt) {
        cv::putText(img, str, pt, cv::FONT_HERSHEY_PLAIN, 1.0, CV_RGB(255, 0, 0), 1, CV_AA);
    };

    // show the whole scene with every marker outlined
    cv::Mat dbgSceneBGR;

    cv::cvtColor(sceneRGB, dbgSceneBGR, CV_RGB2BGR);

    for (std::size_t i = 0; i < quads.size(); i++) {
        // first draw all the lines, then captions
        for (int j = 0; j < 4; j++) {
            cv::line(dbgSceneBGR, quads[i][j], quads[i][(j + 1) % 4], CV_RGB(0, 255, 0), 2);
        }
        for (int j = 0; j < 4; j++) {
            putText(dbgSceneBGR, std::to_string(j), quads[i][j]);
        }
    }

    cv::imshow("Debug scene", dbgSceneBGR);

    // show each marker after warping
    if (markerImgs.size() == 0) return;

    int numImages = markerImgs.size();
    int imageSize = markerImgs[0].rows;

    cv::Mat dbgMarkers{ imageSize * numImages, imageSize, CV_8UC3 };

    for (int i = 0; i < numImages; i++) {
        cv::Rect roi { 0, i*imageSize, imageSize, imageSize };
        cv::Mat markerImgBGR;
        auto img = dbgMarkers(roi);

        cv::cvtColor(markerImgs[i], markerImgBGR, CV_GRAY2BGR);

        markerImgBGR.copyTo(img);

        auto id = std::to_string(markers[i].marker.id);
        putText(img, "id", { 10, 20 });
        putText(img, id, { 70, 20 });

        auto center = 0.25 * std::accumulate(std::begin(quads[i]), std::end(quads[i]), cv::Point2f{ 0.0f, 0.0f });
        auto pos = std::to_string(int(center.x)) + ", " + std::to_string(int(center.y));
        putText(img, "pos", { 10, 40 });
        putText(img, pos, { 70, 40 });

        auto score = std::to_string(markers[i].score);
        putText(img, "score", { 10, 60 });
        putText(img, score, { 70, 60 });
    }
    
    cv::imshow("Debug markers", dbgMarkers);
}


std::vector<MarkerScore> recognizeMarkers(const Camera& camera, const cv::Mat& sceneRGB) {
    std::vector<MarkerScore> markers;
    cv::Mat sceneGrey;

    cv::cvtColor(sceneRGB, sceneGrey, CV_RGB2GRAY);

    auto contours = findContours(sceneGrey, MIN_CONTOUR_LEN);
    auto quads = getPreciseQuads(sceneGrey, contours, MIN_QUAD_AREA);
    auto markerImages = undistortMarkerImages(sceneGrey, quads, NORMALIZED_MARKER_SIZE);

    rotateMarkers(markerImages, quads);
    filterInvalidMarkers(markerImages, quads, VALID_MARKER_TOLERANCE);

    for (std::size_t i = 0; i < markerImages.size(); i++) {

        int     id = getId(markerImages[i]);
        double  score = calculateScore(markerImages[i], id);
        auto    trans = calculateTransformation(camera, quads[i]);

        markers.push_back({ Marker(id, trans.t, trans.r), score });
    }

#ifdef DEBUG_MARKERS
    debugMarkers(sceneRGB, markerImages, markers, quads);
#endif

    return markers;
}

