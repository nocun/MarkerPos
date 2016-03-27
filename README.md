# MarkerPos

*Marker rendering and pose recognition.*

## Overview

MarkerPos consists of 2 parts:

1. *Marker rendering* - creates a 3D environment by using real world camera parameters and renders a marker with the given pose.
2. *Marker recognition* - the rendered image is being used as an input to the recognition module in order to find marker's 3D pose in the camera space.


### Camera parameters

We need 6 parameters in order to create the simplest camera in our imaginary environment:
- sensor resolution (width and height)
- focal length (horizontal and vertical)
- principal point coordinates (X and Y)

### Marker

A marker is just a flat square image that consists of 2 parts:
- a white frame along the edges
- an array of N*N smaller black-or-white squares

![Marker](imgs/marker.png?raw=true)

**Figure 1.** An example of a marker with N=3 (ID=4).

Four corner squares (3 white and 1 black) are used to calculate marker rotation (0, 90, 180, 270 degrees), the rest make up its ID number. N can be in the range [2; 6]. N=2 means there is no ID (2\*2=4 - only four corner squares), N=6 means the ID is a 32-bit number (6\*6-4=32).

![Marker description](imgs/marker_desc.png?raw=true)

**Figure 2.** A marker in a standard upright position. Corner squares are marked with 'X', numbers 0-4 denote ID bit numbers (0 being the [LSB](https://en.wikipedia.org/wiki/Least_significant_bit)).

## Details

The entire recognition algorithm is just a solution to the [P4P](https://en.wikipedia.org/wiki/Perspective-n-Point) problem (it currently uses the cv::solvePNP function). Real camera is being simulated by calculating the OpenGL perspective matrix from camera's intrinsic parameters.
Marker 3D position values are the same as OpenGL coordinates. Rotation matrix is decomposed into Euler angles, we don't use angles outside of (-90; 90), except for rotating around OZ.

The orientation in OpenGL is as follows:
- X increases to the right
- Y increases upwards
- Z increases towards us (we look at negative Z)

## Benchmarks

The following plots show absolute erros between real marker pose and the measured one.
In images 1-3 marker distance from the camera is locked at 3.0.

1. Moving along X and Y axes

    ![XY translation](imgs/translation.png?raw=true)

2. Rotating around OX

    ![OX rotation](imgs/rotation_ox.png?raw=true)

3. Rotating around OZ

    ![OZ rotation](imgs/rotation_oz.png?raw=true)

4. Marker distance varies

    ![XY translation vs. Z](imgs/translation_vs_z.png?raw=true)

## Building

To build the project you will need:
- CMake
- C++11-enabled compiler
- OpenGL 2.1+
- [FreeGLUT](http://freeglut.sourceforge.net/) - tested on 2.8.1
- [OpenCV](http://opencv.org/) - tested on 2.4.8

---

Copyright 2016 Łukasz Nocuń
