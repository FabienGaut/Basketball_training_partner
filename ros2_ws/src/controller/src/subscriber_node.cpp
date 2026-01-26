#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/point.hpp"
#include "basketball_detection/Config.hpp"



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
        RCLCPP_INFO(this->get_logger(), "Robot controller node started");
        RCLCPP_INFO(this->get_logger(), "Subscribed to /basketball_player topic");
    }

private:
    int frameWidth;
    int frameHeight;
    void player_callback(const geometry_msgs::msg::Point::SharedPtr msg)
    {
        double player_x = msg->x;
        double player_y = msg->y;

        RCLCPP_INFO(this->get_logger(), "Player detected at (%.1f, %.1f)", player_x, player_y);

        
        
        process_player_position(player_x, player_y);
    }

    void process_player_position(double x, double y)
    {


        // TODO: Implémenter la logique de contrôle du robot
        // - Calcul de l'angle pour les servomoteurs
        // - Calcul de la vitesse/direction pour les moteurs
        double target = msg->x / framewidth;  // Normaliser la position x entre 0 et 1
        if target < 0.4 {
            RCLCPP_INFO(this->get_logger(), "Turn Left");
            //TODO : Servo / Moteur gauche
        } else if target > 0.6 {
            RCLCPP_INFO(this->get_logger(), "Turn Right");
            //TODO : Servo / Moteur droite
        }
        // TODO: Ajouter ici la logique de contrôle
        // Exemple de calculs à implémenter :

        // 1. Calcul de l'angle horizontal pour orienter le robot vers le joueur
        // double angle = calculate_servo_angle(x);

        // 2. Calcul de la distance estimée (si z disponible ou via taille bbox)
        // double distance = estimate_distance(y);

        // 3. Commande des servomoteurs
        // send_servo_command(angle);

        // 4. Commande des moteurs de déplacement
        // send_motor_command(direction, speed);

        (void)x;  // Supprime le warning unused
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
