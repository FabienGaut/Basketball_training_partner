# Servo - test pigpio

## Compilation

```bash
g++ -o servo_test src/main.cpp -lpigpio -lpthread
```

## Execution (requires root for GPIO)

```bash
sudo ./servo_test
```
