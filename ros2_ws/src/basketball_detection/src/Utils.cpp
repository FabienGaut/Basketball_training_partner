#include "basketball_detection/Utils.hpp"

/**
 * Vérifie si un point (px, py) se trouve à l'intérieur d'une boîte rectangulaire.
 *
 * Cette fonction est utilisée pour déterminer si une personne est un "basketball player" :
 * - On calcule le centre du ballon détecté (cx, cy)
 * - On vérifie si ce centre se trouve dans la bounding box de la personne
 * - Si oui, la personne est considérée comme tenant/possédant le ballon → "basketball player"
 *
 * @param px, py  Coordonnées du point à tester (ex: centre du ballon)
 * @param x1, y1  Coin supérieur gauche de la boîte (ex: bounding box de la personne)
 * @param x2, y2  Coin inférieur droit de la boîte
 * @return true si le point est dans la boîte, false sinon
 */
bool pointInBox(int px, int py, int x1, int y1, int x2, int y2) {
    return px >= x1 && px <= x2 && py >= y1 && py <= y2;
}

std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, last - first + 1);
}
