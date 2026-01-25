#include "basketball_detection/Config.hpp"
#include "basketball_detection/Utils.hpp"
#include <fstream>
#include <iostream>

Config parseConfig(const std::string& filename) {
    Config config;
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Cannot open config file: " << filename << std::endl;
        return config;
    }

    std::string line, currentSection;
    while (std::getline(file, line)) {
        line = trim(line);

        if (line.empty() || line[0] == '#' || line[0] == ';') continue;

        if (line[0] == '[' && line.back() == ']') {
            currentSection = line.substr(1, line.size() - 2);
            continue;
        }

        size_t eqPos = line.find('=');
        if (eqPos != std::string::npos) {
            std::string key = trim(line.substr(0, eqPos));
            std::string value = trim(line.substr(eqPos + 1));

            if (currentSection == "default") {
                if (key == "confidence_threshold") config.confidenceThreshold = std::stof(value);
                else if (key == "webcam_index") config.webcamIndex = std::stoi(value);
                else if (key == "PERSON_MODEL_PATH") config.personModelPath = value;
                else if (key == "BASKET_MODEL_PATH") config.basketModelPath = value;
                else if (key == "FRAME_WIDTH") config.frameWidth = std::stoi(value);
                else if (key == "log_level") config.logLevel = value;
            }
            else if (currentSection == "Visualisation") {
                if (key == "draw_balls") config.drawBalls = (value == "true" || value == "True" || value == "1");
                else if (key == "draw_players") config.drawPlayers = (value == "true" || value == "True" || value == "1");
            }
        }
    }

    return config;
}
