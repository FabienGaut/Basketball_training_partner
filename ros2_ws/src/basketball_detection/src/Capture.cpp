#include "basketball_detection/Capture.hpp"
#include "basketball_detection/Utils.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>

void capture(const Config& config, YOLODetector& personDetector, YOLODetector& basketDetector, PlayerCallback onPlayerDetected) {
    cv::VideoCapture cap(config.webcamIndex, cv::CAP_V4L2);
    cap.set(cv::CAP_PROP_BUFFERSIZE, 1);

    if (!cap.isOpened()) {
        throw std::runtime_error("Webcam non accessible");
    }

    std::cout << "Webcam opened, starting detection..." << std::endl;

    // FPS calculation variables
    auto prevTime = std::chrono::high_resolution_clock::now();
    double fps = 0.0;
    int frameCount = 0;
    const int fpsUpdateInterval = 10; // Update FPS every 10 frames

    while (true) {
        cv::Mat frame;
        cap >> frame;

        if (frame.empty()) break;

        // Calculate FPS
        frameCount++;
        if (frameCount >= fpsUpdateInterval) {
            auto currentTime = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed = currentTime - prevTime;
            fps = frameCount / elapsed.count();
            prevTime = currentTime;
            frameCount = 0;
        }

        // ========= INFERENCE =========
        // Detect persons (class 0 in COCO)
        std::vector<Detection> persons = personDetector.detect(frame, {0});

        // Detect balls (class 32 = sports ball in COCO, or class 0 for custom model)
        std::vector<Detection> balls = basketDetector.detect(frame, {});

        // ========= DRAW BALLS =========
        if (config.drawBalls) {
            for (const auto& ball : balls) {
                int cx = (ball.x1 + ball.x2) / 2;
                int cy = (ball.y1 + ball.y2) / 2;

                cv::rectangle(frame, cv::Point(ball.x1, ball.y1),
                             cv::Point(ball.x2, ball.y2), cv::Scalar(0, 165, 255), 2);
                cv::circle(frame, cv::Point(cx, cy), 4, cv::Scalar(0, 165, 255), -1);

                std::ostringstream label;
                label << "ball " << std::fixed << std::setprecision(2) << ball.confidence;
                cv::putText(frame, label.str(), cv::Point(ball.x1, ball.y1 - 6),
                           cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 165, 255), 2);
            }
        }

        // ========= DRAW PERSONS / PLAYERS =========
        // Logique de classification person vs basketball player :
        // Pour chaque personne détectée, on vérifie si le centre d'un ballon
        // se trouve à l'intérieur de sa bounding box. Si c'est le cas,
        // on considère que cette personne "possède" le ballon et on la
        // reclassifie en "basketball player" (affichée en vert au lieu de bleu).
        if (config.drawPlayers) {
            for (const auto& person : persons) {
                std::string label = "person";
                cv::Scalar color(255, 0, 0);  // Bleu par défaut pour une personne simple

                
                for (const auto& ball : balls) {
                    // Calcul du centre du ballon
                    int cx = (ball.x1 + ball.x2) / 2;
                    int cy = (ball.y1 + ball.y2) / 2;

                    // Si le centre du ballon est dans la bounding box de la personne,
                    // cette personne est considérée comme un "basketball player"
                    if (pointInBox(cx, cy, person.x1, person.y1, person.x2, person.y2)) {
                        label = "basketball player";
                        color = cv::Scalar(0, 255, 0);  // Vert pour un joueur avec ballon

                        if (onPlayerDetected) {
                            onPlayerDetected(person);
                        }

                        break;  // Pas besoin de vérifier les autres ballons
                    }
                }

                cv::rectangle(frame, cv::Point(person.x1, person.y1),
                             cv::Point(person.x2, person.y2), color, 2);

                std::ostringstream labelText;
                labelText << label << " " << std::fixed << std::setprecision(2) << person.confidence;
                cv::putText(frame, labelText.str(), cv::Point(person.x1, person.y1 - 8),
                           cv::FONT_HERSHEY_SIMPLEX, 0.6, color, 2);
            }
        }

        // ========= DRAW FPS =========
        std::ostringstream fpsText;
        fpsText << "C++ | FPS: " << std::fixed << std::setprecision(1) << fps;
        cv::putText(frame, fpsText.str(), cv::Point(10, 30),
                   cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 255, 0), 2);

        //cv::imshow("Basketball Detection (C++)", frame);

        if (cv::waitKey(1) == 'q') break;
    }

    cap.release();
    cv::destroyAllWindows();
}
