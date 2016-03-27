
#include "Processing.h"
#include "Camera.h"
#include "Marker.h"
#include "Rendering.h"
#include "Recognition.h"
#include "Util.h"

#include <GL/freeglut.h>
#include <iomanip>


const double NEAR_PLANE     = 0.1;
const double FAR_PLANE      = 100.0;

const double MOVE_DELTA     = 0.15;
const double ROTATE_ANGLE   = 10.0;

const int    TEXTURE_SIZE   = 256;


Camera camera;
Marker marker;
Transformation origin;
GLuint markerTexture;


// GLUT handlers
void display();
void reshapeHandler(GLsizei width, GLsizei height);
void specialKeysHandler(int key, int, int);
void normalKeysHandler(unsigned char key, int, int);

void compareMarkers(const Marker& marker, const Marker& recognized, double score);



void initializeGL(const Camera& camera, const Marker& marker) {

    ::camera = camera;
    ::marker = marker;

    ::origin = { marker.t, marker.r };

    // GLUT setup
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE);
    glutInitWindowSize(camera.imageWidth, camera.imageHeight);
    glutCreateWindow("MarkerPos");

    glutSetOption(GLUT_MULTISAMPLE, 32);
    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

    // handlers
    glutDisplayFunc(display);
    glutReshapeFunc(reshapeHandler);
    glutKeyboardFunc(normalKeysHandler);
    glutSpecialFunc(specialKeysHandler);
    
    // OpenGL setup
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    glViewport(0, 0, camera.imageWidth, camera.imageHeight);

    glMatrixMode(GL_PROJECTION);
    loadPerspectiveMatrix(camera, NEAR_PLANE, FAR_PLANE);
    glMatrixMode(GL_MODELVIEW);

    ::markerTexture = createMarkerTexture(marker, TEXTURE_SIZE);
}


void finalizeGL() {
    glDeleteTextures(1, &markerTexture);
}


void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    render(marker, markerTexture);
    
    auto sceneImg = getRenderedView(camera.imageWidth, camera.imageHeight);
    auto result = recognizeMarkers(camera, sceneImg);

    if (!result.empty()) {
        compareMarkers(marker, result[0].marker, result[0].score);
    }

	glutSwapBuffers();
}


void compareMarkers(const Marker& marker, const Marker& recognized, double score) {
    auto coutFlags = std::cout.flags();
    std::cout << std::setprecision(2) << std::fixed;

    auto printMarkerInfo = [](const Marker& m) {
        std::cout <<
            "id=" << m.id << "\t"
            "T=(" << m.t.x << ", " << m.t.y << ", " << m.t.z << ")\t"
            "R=(" << m.r.ox << " " << m.r.oy << " " << m.r.oz << ")\n";
    };

    std::cout << "\nOriginal\t";
    printMarkerInfo(marker);

    std::cout << "Recognized\t";
    printMarkerInfo(recognized);

    // Meta information
    std::cout <<
        "Score\t\t" << int(score*100.0) << "%\t"
        "DT=" << distance(marker.t, recognized.t) << "\t\t\t"
        "DR=" << maxAngleDiff(marker.r, recognized.r) << " deg\n";

    std::cout.flags(coutFlags);
}


void reshapeHandler(GLsizei width, GLsizei height) {

    // keep the user from breaking our viewport
    if (width != camera.imageWidth || height != camera.imageHeight) {
        glutReshapeWindow(camera.imageWidth, camera.imageHeight);
    }
}


void specialKeysHandler(int key, int, int) {
	switch (key) {

        // X-axis translation
	    case GLUT_KEY_LEFT:
            marker.t.x -= MOVE_DELTA;
		    break;
	    case GLUT_KEY_RIGHT:
            marker.t.x += MOVE_DELTA;
		    break;

        // Y-axis translation
	    case GLUT_KEY_UP:
            marker.t.y += MOVE_DELTA;
		    break;
	    case GLUT_KEY_DOWN:
            marker.t.y -= MOVE_DELTA;
		    break;

        default:
            return;
	}

	glutPostRedisplay();
}


void normalKeysHandler(unsigned char key, int, int) {
    switch (key) {

        // ESC
	    case 27:
            glutLeaveMainLoop();
            return;

        // Z-axis translation
        case '-':
        case '_':
            marker.t.z -= MOVE_DELTA;
            break;
        case '=':
        case '+':
            marker.t.z += MOVE_DELTA;
            break;

        // OX and OY rotations are locked between -90.0 and 90.0 deg
        // OX rotation
        case 'q':
            if (marker.r.ox < 90.0)
            marker.r.ox = clampAngle(marker.r.ox + ROTATE_ANGLE);
            break;
        case 'a':
            if (marker.r.ox > -90.0)
            marker.r.ox = clampAngle(marker.r.ox - ROTATE_ANGLE);
            break;


        // OY rotation
        case 'w':
            if (marker.r.oy < 90.0)
            marker.r.oy = clampAngle(marker.r.oy + ROTATE_ANGLE);
            break;
        case 's':
            if (marker.r.oy > -90.0)
            marker.r.oy = clampAngle(marker.r.oy - ROTATE_ANGLE);
            break;


        // OZ rotation
        case 'e':
            marker.r.oz = clampAngle(marker.r.oz + ROTATE_ANGLE);
            break;
        case 'd':
            marker.r.oz = clampAngle(marker.r.oz - ROTATE_ANGLE);
            break;

        // reset values to default
        case '1':
            marker.r.ox = origin.r.ox;
            break;

        case '2':
            marker.r.oy = origin.r.oy;
            break;

        case '3':
            marker.r.oz = origin.r.oz;
            break;

        case '4':
            marker.t = origin.t;
            break;

        default:
            return;
	}

	glutPostRedisplay();
}

