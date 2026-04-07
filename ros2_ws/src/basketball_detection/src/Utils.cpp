#include "basketball_detection/Utils.hpp"

/** Check if point (px, py) is inside the given bounding box. */
bool pointInBox(int px, int py, int x1, int y1, int x2, int y2) {
    return px >= x1 && px <= x2 && py >= y1 && py <= y2;
}

std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, last - first + 1);
}
