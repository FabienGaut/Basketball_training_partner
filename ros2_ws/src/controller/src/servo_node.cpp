#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/point.hpp"
#include "Config.hpp"

#include <wiringPi.h>
#include <softPwm.h>

static constexpr int SERVO_PIN     = 18;
static constexpr int PWM_PERIOD    = 200;  // 200 * 100 µs = 20 ms
static constexpr int SERVO_MIN_PWM = 5;    // ~0.5 ms pulse  (~0°)
static constexpr int SERVO_MAX_PWM = 25;   // ~2.5 ms pulse  (~180°)

class ServoController : public rclcpp::Node
{
public:
    ServoController() : Node("servo_controller")
    {
        this->declare_parameter<std::string>("config_path", "/workspace/ball_detection/config/config.ini");
        std::string config_path = this->get_parameter("config_path").as_string();
        Config config = parseConfig(config_path);
        frame_width_ = config.frameWidth;

        if (wiringPiSetupGpio() == -1) {
            RCLCPP_FATAL(this->get_logger(), "wiringPiSetupGpio() failed");
            throw std::runtime_error("wiringPi init failed");
        }

        softPwmCreate(SERVO_PIN, SERVO_MIN_PWM, PWM_PERIOD);
        RCLCPP_INFO(this->get_logger(), "Servo initialized on GPIO %d", SERVO_PIN);

        subscription_ = this->create_subscription<geometry_msgs::msg::Point>(
            "basketball_player",
            10,
            std::bind(&ServoController::player_callback, this, std::placeholders::_1)
        );

        RCLCPP_INFO(this->get_logger(), "Servo controller node started, subscribed to /basketball_player");
    }

    ~ServoController()
    {
        softPwmWrite(SERVO_PIN, SERVO_MIN_PWM);
        softPwmStop(SERVO_PIN);
    }

private:
    int frame_width_;
    rclcpp::Subscription<geometry_msgs::msg::Point>::SharedPtr subscription_;

    void player_callback(const geometry_msgs::msg::Point::SharedPtr msg)
    {
        double x = msg->x;

        // Map x in [0, frame_width_] to PWM in [SERVO_MIN_PWM, SERVO_MAX_PWM]
        double ratio = std::clamp(x / static_cast<double>(frame_width_), 0.0, 1.0);
        int pwm_value = static_cast<int>(SERVO_MIN_PWM + ratio * (SERVO_MAX_PWM - SERVO_MIN_PWM));

        softPwmWrite(SERVO_PIN, pwm_value);

        RCLCPP_INFO(this->get_logger(),
            "Player at x=%.1f -> servo PWM=%d", x, pwm_value);
    }
};

int main(int argc, char* argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<ServoController>());
    rclcpp::shutdown();
    return 0;
}
