#ifndef CAPTURE_HPP
#define CAPTURE_HPP

#include "basketball_detection/Config.hpp"
#include "basketball_detection/YOLODetector.hpp"
#include "basketball_detection/Detection.hpp"
#include <functional>

// Callback appelé quand un joueur de basket est détecté (personne avec ballon)
using PlayerCallback = std::function<void(const Detection&)>;

void capture(const Config& config,
             YOLODetector& personDetector,
             YOLODetector& basketDetector,
             PlayerCallback onPlayerDetected = nullptr);

#endif // CAPTURE_HPP
