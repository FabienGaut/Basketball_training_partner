#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/point.hpp"
#include "Config.hpp"



class RobotController : public rclcpp::Node
{
public:
    RobotController() : Node("robot_controller")
    {
        subscription_ = this->create_subscription<geometry_msgs::msg::Point>(
            "basketball_player",
            10,
            std::bind(&RobotController::player_callback, this, std::placeholders::_1)
        );

        this->declare_parameter<std::string>("config_path", "/workspace/ball_detection/config/config.ini");
        std::string config_path = this->get_parameter("config_path").as_string();
        Config config = parseConfig(config_path);
        frameHeight = config.frameHeight;
        frameWidth = config.frameWidth;
        RCLCPP_INFO(this->get_logger(), "Robot controller node started, subscribed to /basketball_player");
    }

private:
    int frameWidth;
    int frameHeight;
    void player_callback(const geometry_msgs::msg::Point::SharedPtr msg)
    {
        process_player_position(msg->x, msg->y);
    }

    void process_player_position(double x, double y)
    {
        double target = x / frameWidth;
        if (target < 0.4) {
            RCLCPP_INFO(this->get_logger(), "Turn Left");
        } else if (target > 0.6) {
            RCLCPP_INFO(this->get_logger(), "Turn Right");
        }
        // TODO : Implémenter servo+moteurs
        // TODO: moteurs de deplacement
        (void)y;
    }

    rclcpp::Subscription<geometry_msgs::msg::Point>::SharedPtr subscription_;
};

int main(int argc, char* argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<RobotController>());
    rclcpp::shutdown();
    return 0;
}
