#ifndef DETECTION_HPP
#define DETECTION_HPP

struct Detection {
    int x1, y1, x2, y2;
    float confidence;
    int classId;
};

#endif // DETECTION_HPP
