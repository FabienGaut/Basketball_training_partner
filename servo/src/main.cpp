#include <wiringPi.h>
#include <softPwm.h>
#include <iostream>
#include <termios.h>
#include <unistd.h>

#define SERVO_PIN 18
#define PWM_PERIOD 200
#define STEP 5

// 0° -> 5 (0.5ms), 180° -> 25 (2.5ms)
static int angleToPwm(int angle) {
    return 5 + angle * 20 / 180;
}

static int readKey() {
    int ch = getchar();
    if (ch == 27) {
        ch = getchar();
        if (ch == '[') {
            ch = getchar();
            if (ch == 'C') return 1;
            if (ch == 'D') return -1;
        }
    }
    if (ch == 'q') return 0;
    return -2;
}

int main() {
    if (wiringPiSetupGpio() == -1) {
        std::cerr << "Failed to initialize wiringPi" << std::endl;
        return 1;
    }

    softPwmCreate(SERVO_PIN, 0, PWM_PERIOD);

    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    int angle = 90;
    softPwmWrite(SERVO_PIN, angleToPwm(angle));
    std::cout << "Angle: " << angle << "°  [left/right arrows, q to quit]" << std::endl;

    while (true) {
        int key = readKey();
        if (key == 0) break;
        if (key == -2) continue;

        angle += key * STEP;
        if (angle < 0) angle = 0;
        if (angle > 180) angle = 180;

        softPwmWrite(SERVO_PIN, angleToPwm(angle));
        std::cout << "\rAngle: " << angle << "°   " << std::flush;
    }

    softPwmWrite(SERVO_PIN, 0);
    softPwmStop(SERVO_PIN);
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    std::cout << std::endl;
    return 0;
}
