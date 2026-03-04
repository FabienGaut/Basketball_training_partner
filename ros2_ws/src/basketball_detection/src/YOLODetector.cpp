#include "basketball_detection/YOLODetector.hpp"
#include <iostream>
#include <algorithm>
#include <cstring>

YOLODetector::YOLODetector(const std::string& modelPath, float confThreshold)
    : confThreshold_(confThreshold), env_(ORT_LOGGING_LEVEL_WARNING, "YOLODetector") {

    Ort::SessionOptions sessionOptions; //  objet de configuration pour la session d'inférence ONNX Runtime. 
    sessionOptions.SetIntraOpNumThreads(4); // nombre de threads a utiliser, a adapter selon le CPU
    sessionOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
    /*
    Avec ORT_ENABLE_ALL, ONNX Runtime va :

        Fusionner des couches consécutives
        Éliminer les calculs redondants
        Optimiser la disposition mémoire
        Appliquer des optimisations spécifiques au CPU/GPU détecté
*/

    session_ = std::make_unique<Ort::Session>(env_, modelPath.c_str(), sessionOptions);

    // Get input info
    Ort::AllocatorWithDefaultOptions allocator; // Crée un allocateur mémoire par défaut
    auto inputName = session_->GetInputNameAllocated(0, allocator);// Récupère le nom de l'entrée à l'index 0 (la première entrée du modèle).
    inputName_ = inputName.get();

    auto inputShape = session_->GetInputTypeInfo(0).GetTensorTypeAndShapeInfo().GetShape();
    inputHeight_ = (inputShape[2] > 0) ? static_cast<int>(inputShape[2]) : 640;
    inputWidth_ = (inputShape[3] > 0) ? static_cast<int>(inputShape[3]) : 640;

    // Get output info
    auto outputName = session_->GetOutputNameAllocated(0, allocator);
    outputName_ = outputName.get();

    std::cout << "Model loaded: " << modelPath << std::endl;
    std::cout << "  Input size: " << inputWidth_ << "x" << inputHeight_ << std::endl;
}

std::vector<Detection> YOLODetector::detect(const cv::Mat& frame, const std::vector<int>& filterClasses) {
    std::vector<Detection> detections;

    // Preprocess
    cv::Mat resized, blob;
    cv::resize(frame, resized, cv::Size(inputWidth_, inputHeight_));
    resized.convertTo(blob, CV_32F, 1.0 / 255.0);

    // HWC (OpenCV) to CHW : Channels × Height × Width , format pytorch, yolo, ...
    std::vector<cv::Mat> channels(3);
    cv::split(blob, channels);

    std::vector<float> inputTensor(3 * inputHeight_ * inputWidth_);
    size_t channelSize = inputHeight_ * inputWidth_;
    std::memcpy(inputTensor.data(), channels[0].data, channelSize * sizeof(float));
    std::memcpy(inputTensor.data() + channelSize, channels[1].data, channelSize * sizeof(float));
    std::memcpy(inputTensor.data() + 2 * channelSize, channels[2].data, channelSize * sizeof(float));

    // Create input tensor (tableau multidimensionnel de nombres)
    std::vector<int64_t> inputShape = {1, 3, inputHeight_, inputWidth_};
    Ort::MemoryInfo memInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    Ort::Value inputOrt = Ort::Value::CreateTensor<float>(
        memInfo, inputTensor.data(), inputTensor.size(), inputShape.data(), inputShape.size());
/*
tensor 4D de shape [1, 3, 640, 640] :
    1         →  Batch size (1 seule image)
       3         →  Canaux (R, G, B)
       640       →  Hauteur
       640       →  Largeur
*/
    // Run inference : l'inference correspond a l'execution du modele sur les donnees d'entree pour obtenir les resultats de sortie.
    const char* inputNames[] = {inputName_.c_str()};
    const char* outputNames[] = {outputName_.c_str()};

    auto outputs = session_->Run(Ort::RunOptions{nullptr}, inputNames, &inputOrt, 1, outputNames, 1);

    // Parse output
    float* outputData = outputs[0].GetTensorMutableData<float>();
    auto outputShape = outputs[0].GetTensorTypeAndShapeInfo().GetShape();

    float xFactor = static_cast<float>(frame.cols) / inputWidth_;
    float yFactor = static_cast<float>(frame.rows) / inputHeight_;

    // YOLOv8 output format: [1, 84, 8400] or [1, N, 6] for end2end
    if (outputShape.size() == 3) {
        if (outputShape[2] == 6) {
            // End2end format: [1, N, 6] where 6 = [x1, y1, x2, y2, conf, class]
            int numDetections = static_cast<int>(outputShape[1]);

            for (int i = 0; i < numDetections; ++i) {
                float* det = outputData + i * 6;
                float conf = det[4];

                if (conf < confThreshold_) continue;

                int classId = static_cast<int>(det[5]);

                if (!filterClasses.empty()) {
                    bool found = std::find(filterClasses.begin(), filterClasses.end(), classId) != filterClasses.end();
                    if (!found) continue;
                }

                Detection d;
                d.x1 = static_cast<int>(det[0] * xFactor);
                d.y1 = static_cast<int>(det[1] * yFactor);
                d.x2 = static_cast<int>(det[2] * xFactor);
                d.y2 = static_cast<int>(det[3] * yFactor); //coordonnées bounding box
                d.confidence = conf;
                d.classId = classId;
                detections.push_back(d);
            }
        } else {
            // Standard format: [1, 84, 8400] -> transpose to [8400, 84]
            int dimensions = static_cast<int>(outputShape[1]);
            int rows = static_cast<int>(outputShape[2]);

            std::vector<int> classIds;
            std::vector<float> confidences;
            std::vector<cv::Rect> boxes;

            for (int i = 0; i < rows; ++i) {
                float cx = outputData[0 * rows + i];
                float cy = outputData[1 * rows + i];
                float w = outputData[2 * rows + i];
                float h = outputData[3 * rows + i];

                float maxScore = 0;
                int maxClassId = 0;
                for (int j = 4; j < dimensions; ++j) {
                    float score = outputData[j * rows + i];
                    if (score > maxScore) {
                        maxScore = score;
                        maxClassId = j - 4;
                    }
                }

                if (maxScore < confThreshold_) continue;

                if (!filterClasses.empty()) {
                    bool found = std::find(filterClasses.begin(), filterClasses.end(), maxClassId) != filterClasses.end();
                    if (!found) continue;
                }

                int x1 = static_cast<int>((cx - w / 2) * xFactor);
                int y1 = static_cast<int>((cy - h / 2) * yFactor);
                int width = static_cast<int>(w * xFactor);
                int height = static_cast<int>(h * yFactor);

                boxes.push_back(cv::Rect(x1, y1, width, height));
                confidences.push_back(maxScore);
                classIds.push_back(maxClassId);
            }

            // Apply NMS : NMS (Non-Maximum Suppression) est un algorithme qui élimine les détections redondantes quand plusieurs bounding boxes détectent le même objet.
            std::vector<int> indices;
            cv::dnn::NMSBoxes(boxes, confidences, confThreshold_, 0.45f, indices);

            for (int idx : indices) {
                Detection d;
                d.x1 = boxes[idx].x;
                d.y1 = boxes[idx].y;
                d.x2 = boxes[idx].x + boxes[idx].width;
                d.y2 = boxes[idx].y + boxes[idx].height;
                d.confidence = confidences[idx];
                d.classId = classIds[idx];
                detections.push_back(d);
            }
        }
    }

    return detections;
}
