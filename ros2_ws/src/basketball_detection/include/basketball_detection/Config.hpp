#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>

struct Config {
    float confidenceThreshold = 0.5f;
    int webcamIndex = 0;
    std::string personModelPath;
    std::string basketModelPath;
    int frameWidth = 640;
    int frameHeight = 480;
    bool drawBalls = true;
    bool drawPlayers = true;
    std::string logLevel = "INFO";
};

Config parseConfig(const std::string& filename);

#endif // CONFIG_HPP
