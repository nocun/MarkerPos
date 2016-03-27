
#include "Camera.h"
#include "Marker.h"
#include "Processing.h"

#include <GL/freeglut.h>
#include <iostream>


void printInterfaceInfo();

int main(int argc, char* argv[]) {
    Camera camera;
    Marker marker;

    marker.id = 4;
    
    // OpenGL looks at negative Z by default
    marker.t = { 0.0, 0.0, -3.0 };
    marker.r = { 0.0, 0.0, 0.0 };

    printInterfaceInfo();

    glutInit(&argc, argv);
    initializeGL(camera, marker);

    glutMainLoop();

    finalizeGL();
    return EXIT_SUCCESS;
}


void printInterfaceInfo() {
    std::cout <<
        "MarkerPos - marker rendering and recognition\n\n"
        "Interface:\n"
        "  LEFT, RIGHT - move along OX\n"
        "  UP, DOWN - move along OY\n"
        "  -, + - move along OZ\n"
        "  Q, A - rotate around OX\n"
        "  W, S - rotate around OY\n"
        "  E, D - rotate around OZ\n"
        "  1, 2, 3, 4 - reset rotations and translation\n"
        "  ESC - quit\n\n"
        "DT - Euclidean distance between actual and recognized marker positions\n"
        "DR - maximum difference in angles\n";
}

