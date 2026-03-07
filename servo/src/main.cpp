#include <wiringPi.h>
#include <softPwm.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <termios.h>
#include <sys/select.h>
#include <sys/time.h>

#define SERVO_PIN 18
#define PWM_MIN 5
#define PWM_MAX 25

termios orig_termios;

void reset_terminal_mode()
{
    tcsetattr(0, TCSANOW, &orig_termios);
}

void set_conio_terminal_mode()
{
    termios new_termios;

    tcgetattr(0, &orig_termios);
    new_termios = orig_termios;

    new_termios.c_lflag &= ~(ICANON | ECHO);

    tcsetattr(0, TCSANOW, &new_termios);

    atexit(reset_terminal_mode);
}

int kbhit()
{
    timeval tv = {0L, 0L};
    fd_set fds;

    FD_ZERO(&fds);
    FD_SET(0, &fds);

    return select(1, &fds, NULL, NULL, &tv);
}

int main()
{
    wiringPiSetupGpio();
    softPwmCreate(SERVO_PIN, 15, 200);

    set_conio_terminal_mode();

    int pwm = 15;
    char c;

    printf("Controle servo : fleche gauche/droite, q pour quitter\n");

    while (true)
    {
        if (kbhit())
        {
            c = getchar();

            if (c == 27) // séquence flèche
            {
                getchar();
                c = getchar();

                if (c == 'C') pwm++; // droite
                if (c == 'D') pwm--; // gauche
            }

            if (c == 'q')
                break;

            if (pwm < PWM_MIN) pwm = PWM_MIN;
            if (pwm > PWM_MAX) pwm = PWM_MAX;

            softPwmWrite(SERVO_PIN, pwm);

            printf("\rPWM: %d ", pwm);
            fflush(stdout);
        }

        usleep(10000);
    }

    return 0;
}