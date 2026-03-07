#include <pigpio.h>
#include <iostream>
#include <termios.h>
#include <unistd.h>

#define SERVO_PIN 18
#define STEP 5

// 500-2500 µs -> 0-180°
static int angleToPulse(int angle) {
    return 500 + angle * 2000 / 180;
}

static int readKey() {
    int ch = getchar();
    if (ch == 27) {
        ch = getchar();
        if (ch == '[') {
            ch = getchar();
            if (ch == 'C') return 1;  // right arrow
            if (ch == 'D') return -1; // left arrow
        }
    }
    if (ch == 'q') return 0;
    return -2;
}

int main() {
    if (gpioInitialise() < 0) {
        std::cerr << "Failed to initialize pigpio\n";
        return 1;
    }

    // raw terminal mode (no enter needed)
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    int angle = 90;
    gpioServo(SERVO_PIN, angleToPulse(angle));
    std::cout << "Angle: " << angle << "°  [left/right arrows, q to quit]\n";

    while (true) {
        int key = readKey();
        if (key == 0) break;
        if (key == -2) continue;

        angle += key * STEP;
        if (angle < 0) angle = 0;
        if (angle > 180) angle = 180;

        gpioServo(SERVO_PIN, angleToPulse(angle));
        std::cout << "\rAngle: " << angle << "°   " << std::flush;
    }

    gpioServo(SERVO_PIN, 0);
    gpioTerminate();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    std::cout << "\n";
    return 0;
}
