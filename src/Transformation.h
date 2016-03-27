#pragma once

struct Translation {
    double x, y, z;
};


struct Rotation {
    double ox, oy, oz;
};


struct Transformation {
    Translation t;
    Rotation r;
};

