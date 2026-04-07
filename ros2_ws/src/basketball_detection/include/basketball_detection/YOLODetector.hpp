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
    YOLODetector(const std::string& modelPath, float confThreshold,
                 int intraOpThreads = 0, int interOpThreads = 0);
    std::vector<Detection> detect(const cv::Mat& frame, const std::vector<int>& filterClasses = {});

private:
    Ort::Env env_;
    std::unique_ptr<Ort::Session> session_;
    std::string inputName_;
    std::string outputName_;
    int inputWidth_;
    int inputHeight_;
    float confThreshold_;

    // Pre-allocated buffers to avoid per-frame allocation
    cv::Mat blob_;
    std::vector<float> inputTensor_;
};

#endif // YOLODETECTOR_HPP
