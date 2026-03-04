#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/point.hpp"
#include "basketball_detection/Config.hpp"
#include "basketball_detection/YOLODetector.hpp"
#include "basketball_detection/Capture.hpp"
#include <iostream>

int main(int argc, char* argv[])
{
    rclcpp::init(argc, argv);

    auto node = rclcpp::Node::make_shared("basketball_publisher");
    node->declare_parameter<std::string>("config_path", "/workspace/ball_detection/config/config.ini");
    auto qos = rclcpp::SensorDataQoS();  // QoS optimisé pour données capteur

    auto publisher = node->create_publisher<geometry_msgs::msg::Point>(
        "basketball_player",
        qos);
        
    try {
        std::string config_path = node->get_parameter("config_path").as_string();
        Config config = parseConfig(config_path);

        RCLCPP_INFO(node->get_logger(), "Configuration loaded");
        RCLCPP_INFO(node->get_logger(), "Loading person model...");
        YOLODetector personDetector(config.personModelPath, config.confidenceThreshold);

        RCLCPP_INFO(node->get_logger(), "Loading basket model...");
        YOLODetector basketDetector(config.basketModelPath, config.confidenceThreshold);

        RCLCPP_INFO(node->get_logger(), "Starting capture...");

        capture(config, personDetector, basketDetector,
            [&publisher, &node](const Detection& player) {
                auto msg = geometry_msgs::msg::Point();
                msg.x = (player.x1 + player.x2) / 2.0;
                msg.y = (player.y1 + player.y2) / 2.0;
                msg.z = 0.0;

                RCLCPP_INFO(node->get_logger(), "Player detected at (%.1f, %.1f)", msg.x, msg.y);
                publisher->publish(msg);
            });

    } catch (const std::exception& e) {
        RCLCPP_ERROR(node->get_logger(), "Error: %s", e.what());
        rclcpp::shutdown();
        return 1;
    }

    rclcpp::shutdown();
    return 0;
}
