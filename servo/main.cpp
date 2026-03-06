#include <wiringPi.h>
#include <softPwm.h>
#include <unistd.h>

int main() {
    wiringPiSetupGpio(); // BCM GPIO numbering
    int servoPin = 18;

    softPwmCreate(servoPin, 0, 200);

    for (int i = 5; i <= 25; i++) { // servo sweep
        softPwmWrite(servoPin, i);
        usleep(20000);
    }

    for (int i = 25; i >= 5; i--) {
        softPwmWrite(servoPin, i);
        usleep(20000);
    }

    return 0;
}