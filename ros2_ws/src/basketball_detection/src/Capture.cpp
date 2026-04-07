#include "basketball_detection/Capture.hpp"
#include "basketball_detection/Utils.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <future>

void capture(const Config& config, YOLODetector& personDetector, YOLODetector& basketDetector, PlayerCallback onPlayerDetected) {
    cv::VideoCapture cap(config.webcamIndex, cv::CAP_V4L2);
    cap.set(cv::CAP_PROP_BUFFERSIZE, 1);
    cap.set(cv::CAP_PROP_FRAME_WIDTH, config.frameWidth);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, config.frameHeight);

    if (!cap.isOpened()) {
        throw std::runtime_error("Webcam not accessible");
    }

    std::cout << "Webcam opened, starting detection..." << std::endl;

    // FPS calculation variables
    auto prevTime = std::chrono::high_resolution_clock::now();
    double fps = 0.0;
    int frameCount = 0;
    const int fpsUpdateInterval = 10;

    // Skip-frame: reuse last detections on skipped frames
    std::vector<Detection> lastPersons;
    std::vector<Detection> lastBalls;
    int skipCounter = 0;

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

        // ========= INFERENCE (with skip-frame) =========
        skipCounter++;
        if (skipCounter >= config.processEveryNFrames) {
            skipCounter = 0;

            // Run both detections in parallel using std::async
            auto personFuture = std::async(std::launch::async, [&]() {
                return personDetector.detect(frame, {0});
            });
            auto basketFuture = std::async(std::launch::async, [&]() {
                return basketDetector.detect(frame, {});
            });

            lastPersons = personFuture.get();
            lastBalls = basketFuture.get();
        }

        // Use last detection results (current or cached from skip-frame)
        const auto& persons = lastPersons;
        const auto& balls = lastBalls;

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
        if (config.drawPlayers) {
            for (const auto& person : persons) {
                std::string label = "person";
                cv::Scalar color(255, 0, 0);


                for (const auto& ball : balls) {
                    int cx = (ball.x1 + ball.x2) / 2;
                    int cy = (ball.y1 + ball.y2) / 2;

                    if (pointInBox(cx, cy, person.x1, person.y1, person.x2, person.y2)) {
                        label = "basketball player";
                        color = cv::Scalar(0, 255, 0);

                        if (onPlayerDetected) {
                            onPlayerDetected(person);
                        }

                        break;
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
