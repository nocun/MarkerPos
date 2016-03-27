
#include "Rendering.h"
#include "Marker.h"
#include "Camera.h"

#include <opencv2/opencv.hpp>
#include <GL/freeglut.h>


void render(const Marker& marker, GLuint textureId) {
    double half = Marker::MARKER_SIZE / 2.0;
    Translation t = marker.t;
    Rotation r = marker.r;

    glPushMatrix();
    glLoadIdentity();

    glTranslated(t.x, t.y, t.z);
    glRotated(r.ox, 1.0, 0.0, 0.0);
    glRotated(r.oy, 0.0, 1.0, 0.0);
    glRotated(r.oz, 0.0, 0.0, 1.0);

    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    glBindTexture(GL_TEXTURE_2D, textureId);

    glBegin(GL_TRIANGLES);
        glTexCoord2d(0.0, 0.0);     glVertex3d(-half, -half, 0.0f);
        glTexCoord2d(1.0, 0.0);     glVertex3d(half, -half, 0.0f);
        glTexCoord2d(1.0, 1.0);     glVertex3d(half, half, 0.0f);

        glTexCoord2d(1.0, 1.0);     glVertex3d(half, half, 0.0f);
        glTexCoord2d(0.0, 1.0);     glVertex3d(-half, half, 0.0f);
        glTexCoord2d(0.0, 0.0);     glVertex3d(-half, -half, 0.0f);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}


GLuint createTexture(int width, int height, const void* pixels) {
    GLuint texture;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);

    return texture;
}


GLuint createMarkerTexture(const Marker& marker, int textureSize) {
    cv::Mat texture{ textureSize, textureSize, CV_8UC3, CV_RGB(255, 255, 255) };

    auto squares = getIdSquares(textureSize);
    auto blackCorner = getSquare(textureSize, UPPER_RIGHT);

    texture(blackCorner) = CV_RGB(0, 0, 0);

    for (int bitIndex = 0; bitIndex < int(squares.size()); bitIndex++) {
        if (~marker.id & (1 << bitIndex)) {
            texture(squares[bitIndex]) = CV_RGB(0, 0, 0);
        }
    }

    // OpenGL likes its textures upside down
    cv::flip(texture, texture, 0);

    return createTexture(texture.cols, texture.rows, texture.data);
}


cv::Mat getRenderedView(int width, int height) {
    cv::Mat view{ height, width, CV_8UC3 };

    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, view.data);
    cv::flip(view, view, 0);

    return view;
}


void loadPerspectiveMatrix(const Camera& camera, GLdouble nearPlane, GLdouble farPlane) {

    // we need to invert principal point's Y
    Camera c = camera;
    c.principalY = double(c.imageHeight) - c.principalY;

    auto width = GLdouble(c.imageWidth);
    auto height = GLdouble(c.imageHeight);

    auto cameraMatrix33 = getCameraMatrix(c);
    auto perspectiveMatrix44 = cv::Mat{ 4, 4, CV_64FC1, 0.0 };

    // negate the 3rd column
    cameraMatrix33.col(2) *= -1.0;

    // copy the top two rows
    cameraMatrix33({ 0, 0, 3, 2 }).copyTo(perspectiveMatrix44({ 0, 0, 3, 2 }));

    // copy the third row
    cameraMatrix33({ 0, 2, 3, 1 }).copyTo(perspectiveMatrix44({ 0, 3, 3, 1 }));

    // set near and far values
    perspectiveMatrix44.at<double>(2, 2) = nearPlane + farPlane;
    perspectiveMatrix44.at<double>(2, 3) = nearPlane * farPlane;

    double L = 0.0, R = width;
    double T = height, B = 0.0;
    double N = nearPlane, F = farPlane;

    double tx = -(R + L) / (R - L);
    double ty = -(T + B) / (T - B);
    double tz = -(F + N) / (F - N);

    // transforming to normalized device coordinates
    cv::Mat NDC = (cv::Mat_<double>(4, 4) <<
        2.0 / (R - L),  0.0,            0.0,            tx,
        0.0,            2.0 / (T - B),  0.0,            ty,
        0.0,            0.0,           -2.0 / (F - N),  tz,
        0.0,            0.0,            0.0,            1.0);

    cv::Mat projectionMatrix = NDC * perspectiveMatrix44;

    GLdouble m[16];

    int i = 0;
    for (int x = 0; x < 4; x++) {
        for (int y = 0; y < 4; y++) {
            m[i++] = projectionMatrix.at<double>(y, x);
        }
    }

    glLoadMatrixd(m);
}


void setPerspective(GLdouble left, GLdouble right,
    GLdouble bottom, GLdouble top,
    GLdouble focalX, GLdouble focalY,
    GLdouble principalX, GLdouble principalY,
    GLdouble nearPlane, GLdouble farPlane) {

    // we need to invert principal point's Y
    GLdouble principalX_ = principalX;
    GLdouble principalY_ = (top - bottom) - principalY;

    GLdouble left_ = (nearPlane / focalX) * (left - principalX_);
    GLdouble right_ = (nearPlane / focalX) * (right - principalX_);

    GLdouble top_ = (nearPlane / focalY) * (top - principalY_);
    GLdouble bottom_ = (nearPlane / focalY) * (bottom - principalY_);

    glFrustum(left_, right_, bottom_, top_, nearPlane, farPlane);
}


void setPerspective(GLdouble width, GLdouble height,
    GLdouble focalX, GLdouble focalY,
    GLdouble principalX, GLdouble principalY,
    GLdouble nearPlane, GLdouble farPlane) {

    setPerspective(0.0, width, 0.0, height, focalX, focalY, principalX, principalY, nearPlane, farPlane);
}

