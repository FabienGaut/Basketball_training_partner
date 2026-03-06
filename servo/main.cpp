#include <wiringPi.h>
#include <softPwm.h>
#include <unistd.h>

int main() {
    wiringPiSetupGpio(); // numérotation GPIO BCM
    int servoPin = 18; // GPIO connecté au servo

    softPwmCreate(servoPin, 0, 200); // min 0 max 200 (approximatif)

    for (int i = 5; i <= 25; i++) { // balayage du servo
        softPwmWrite(servoPin, i);
        usleep(20000);
    }

    for (int i = 25; i >= 5; i--) {
        softPwmWrite(servoPin, i);
        usleep(20000);
    }

    return 0;
}