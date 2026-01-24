#include "Config.hpp"
#include "YOLODetector.hpp"
#include "Capture.hpp"
#include <iostream>

int main(int argc, char** argv) {
    try {
        // Load configuration
        Config config = parseConfig("config/config.ini");

        std::cout << "Configuration loaded:" << std::endl;
        std::cout << "  Confidence threshold: " << config.confidenceThreshold << std::endl;
        std::cout << "  Webcam index: " << config.webcamIndex << std::endl;
        std::cout << "  Person model: " << config.personModelPath << std::endl;
        std::cout << "  Basket model: " << config.basketModelPath << std::endl;

        // Load models
        std::cout << "\nLoading person model..." << std::endl;
        YOLODetector personDetector(config.personModelPath, config.confidenceThreshold);

        std::cout << "\nLoading basket model..." << std::endl;
        YOLODetector basketDetector(config.basketModelPath, config.confidenceThreshold);

        // Start capture
        capture(config, personDetector, basketDetector);

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
