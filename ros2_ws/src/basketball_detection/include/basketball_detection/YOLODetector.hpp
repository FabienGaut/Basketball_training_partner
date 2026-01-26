#ifndef YOLODETECTOR_HPP
#define YOLODETECTOR_HPP

#include <opencv2/opencv.hpp>
#include <onnxruntime_cxx_api.h>
#include <vector>
#include <string>
#include <memory>
#include "Detection.hpp"

class YOLODetector {
public:
    YOLODetector(const std::string& modelPath, float confThreshold);
    std::vector<Detection> detect(const cv::Mat& frame, const std::vector<int>& filterClasses = {});

private:
    Ort::Env env_;
    std::unique_ptr<Ort::Session> session_;
    std::string inputName_;
    std::string outputName_;
    int inputWidth_;
    int inputHeight_;
    float confThreshold_;
};

#endif // YOLODETECTOR_HPP
