#include <pigpio.h>
#include <iostream>
#include <unistd.h>

#define SERVO_PIN 18

int main() {
    if (gpioInitialise() < 0) {
        std::cerr << "Failed to initialize pigpio\n";
        return 1;
    }

    // Position 0°
    gpioServo(SERVO_PIN, 1000); // 1000 µs ≈ 0°
    sleep(2);

    // Position 90°
    gpioServo(SERVO_PIN, 1500); // 1500 µs ≈ 90°
    sleep(2);

    // Position 180°
    gpioServo(SERVO_PIN, 2000); // 2000 µs ≈ 180°
    sleep(2);

    // Stop signal
    gpioServo(SERVO_PIN, 0);

    gpioTerminate();
    return 0;
}