<p align="center">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="assets/logo.png">
    <source media="(prefers-color-scheme: light)" srcset="assets/logo.png">
    <img src="assets/logo.png" width="250" alt="Robot Defender Logo" style="image-rendering: pixelated;">
  </picture>
</p>

# Basketball Player Detection - Robot Defender

Computer vision system for a basketball defender robot. The project detects in real time players holding a ball and transmits the bounding box center coordinates via ROS 2, allowing the robot to position itself as a defender facing the ball carrier.

## Goal

The robot must:
1. **Detect** when a person is holding a basketball (classified as "basketball player")
2. **Retrieve** the bounding box center coordinates of the player
3. **Transmit** these coordinates via ROS 2 to the robot's components
4. **Orient** the servo motor toward the detected player
5. **Move** to stay facing the ball carrier (in progress)

## Detection Principle

The system uses two YOLO models in parallel:

| Model | File | Role |
|-------|------|------|
| **COCO** | `yolo26n` | Person detection (class 0) |
| **Custom** | `ballDetection` | Basketball detection |

**Classification rule**: A person is identified as a **"basketball player"** if the center of a detected ball falls within their bounding box.

## ROS 2 Architecture

```
[ publisher_node ]              [ servo_node ]           [ subscriber_node ]
  basketball_detection            controller               controller
  - Webcam                        - Subscribes /basketball_player
  - YOLO detection                - wiringPi softPwm       - Motor placeholder
  - Publishes /basketball_player  - GPIO 18 -> Servo       - Direction logic
         |                              ^                        ^
         +------------------------------+------------------------+
                    geometry_msgs/Point (x, y)
```

| Node | Package | Role |
|------|---------|------|
| `publisher_node` | `basketball_detection` | YOLO detection + coordinate publishing |
| `servo_node` | `controller` | Servo motor control on GPIO 18 |
| `subscriber_node` | `controller` | Motor control (to be implemented) |

| Topic | Type | Description |
|-------|------|-------------|
| `/basketball_player` | `geometry_msgs/msg/Point` | (x, y) coordinates of the player's bounding box center |

## Available Versions

| Version | Performance | Model Format | Use Case |
|---------|-------------|--------------|----------|
| **ROS 2** | 5-10 FPS | ONNX | Robot integration (recommended) |
| **C++ standalone** | 5-10.4 FPS | ONNX | Testing without ROS |
| **Python** | 4-9.4 FPS | PyTorch | Prototyping |

## Project Structure

```
ball_detection/
├── config/
│   ├── config.ini              # Configuration (Docker paths)
│   └── config_cpp.ini          # Configuration (local paths)
├── cpp/                        # C++ standalone version
│   └── build/
├── ros2_ws/                    # ROS 2 workspace
│   └── src/
│       ├── basketball_detection/   # YOLO detection (publisher)
│       │   ├── CMakeLists.txt
│       │   ├── package.xml
│       │   ├── include/basketball_detection/
│       │   │   ├── Config.hpp
│       │   │   ├── YOLODetector.hpp
│       │   │   ├── Capture.hpp
│       │   │   ├── Detection.hpp
│       │   │   └── Utils.hpp
│       │   └── src/
│       │       ├── publisher_node.cpp
│       │       ├── Config.cpp
│       │       ├── YOLODetector.cpp
│       │       ├── Capture.cpp
│       │       └── Utils.cpp
│       └── controller/             # Robot control (subscribers)
│           ├── CMakeLists.txt
│           ├── package.xml
│           ├── include/
│           │   └── Config.hpp
│           └── src/
│               ├── servo_node.cpp      # Servo control GPIO 18
│               ├── subscriber_node.cpp # Motor control (TODO)
│               └── Config.cpp
├── servo/                      # Standalone servo tests
│   ├── main.cpp                # wiringPi test
│   ├── main.py                 # gpiozero test
│   └── src/
│       └── main.cpp            # pigpio test
├── python/                     # Python version
│   ├── detect.py
│   └── requirements.txt
├── models/                     # YOLO models (.pt and .onnx)
├── docker/
│   ├── dockerfile_cpp
│   ├── dockerfile_python
│   └── compose.yaml
├── utils/
│   └── export_models_to_onnx.py
├── launch_ros.sh               # Docker launch script
├── tests/
├── .gitignore
└── README.md
```

---

## Quick Start (Docker + ROS 2)

This is the recommended method to run the full system.

### Prerequisites

- Docker & Docker Compose
- Webcam
- Raspberry Pi or machine with GPIO (for the servo)

### Automatic Launch

```bash
./launch_ros.sh
```

This script:
1. Starts the Docker container (`ball_detection_ros`) in privileged mode
2. Builds the `basketball_detection` and `controller` packages with colcon
3. Launches `publisher_node` (detection + publishing on `/basketball_player`)
4. Launches `servo_node` (subscribes to `/basketball_player`, controls the servo on GPIO 18)

Press **Ctrl+C** to stop both nodes.

### Manual Launch (step by step)

**1. Start the container**
```bash
docker compose -f docker/compose.yaml up -d
```

**2. Enter the container**
```bash
docker exec -it ball_detection_ros bash
```

**3. Build the workspace**
```bash
source /opt/ros/humble/setup.bash
cd /workspace/ball_detection/ros2_ws
colcon build --packages-select basketball_detection controller
source install/setup.bash
```

**4. Launch the detection node** (terminal 1)
```bash
ros2 run basketball_detection publisher_node \
    --ros-args -p config_path:=/workspace/ball_detection/config/config.ini
```

**5. Launch the servo node** (terminal 2)
```bash
docker exec -it ball_detection_ros bash
source /opt/ros/humble/setup.bash
source /workspace/ball_detection/ros2_ws/install/setup.bash
ros2 run controller servo_node \
    --ros-args -p config_path:=/workspace/ball_detection/config/config.ini
```

### Listen to the topic

```bash
ros2 topic echo /basketball_player
```

---

## Installation - C++ Standalone Version

### Prerequisites

- CMake 3.10+
- C++17 compiler
- OpenCV 4.x
- ONNX Runtime (included in `deps/`)

### Build and Run

```bash
cd cpp
mkdir -p build && cd build
cmake ..
make
./detectBasketBallPlayer
```

---

## Installation - Python Version

### Prerequisites

- Python 3.8+
- Webcam

### Install and Run

```bash
pip install -r python/requirements.txt
python python/detect.py
```

---

## Configuration

File: `config/config.ini`

```ini
[default]
log_level = DEBUG
webcam_index = 0
confidence_threshold = 0.5
PERSON_MODEL_PATH = models/yolo26n.onnx
BASKET_MODEL_PATH = models/ballDetection.onnx
FRAME_WIDTH = 640
FRAME_HEIGHT = 480

[Visualisation]
draw_balls = True
draw_players = True
```

| Parameter | Description | Default |
|-----------|-------------|---------|
| `webcam_index` | Webcam index | `0` |
| `confidence_threshold` | YOLO confidence threshold | `0.5` |
| `FRAME_WIDTH` | Frame width | `640` |
| `FRAME_HEIGHT` | Frame height | `480` |
| `draw_balls` | Display balls | `True` |
| `draw_players` | Display players | `True` |

## Model Conversion

To convert PyTorch models to ONNX (required for C++ and ROS 2):

```bash
python utils/export_models_to_onnx.py
```

## Visual Display

| Color | Meaning |
|-------|---------|
| **Blue** | Person without ball |
| **Green** | Basketball player (person with ball) |
| **Orange** | Detected basketball |

## Servo Motor

The `servo/` directory contains standalone test scripts for the servo motor (GPIO 18):

| File | Library | Usage |
|------|---------|-------|
| `servo/main.cpp` | wiringPi (softPwm) | Servo sweep |
| `servo/src/main.cpp` | pigpio | 0/90/180 degree positions |
| `servo/main.py` | gpiozero | Min/mid/max test |

In production, servo control is handled by the ROS 2 `servo_node` which maps the detected player's X position to the servo angle (0-180 degrees).

## TODO

- [x] Publish data on a ROS 2 topic
- [x] Servo motor subscriber node (`servo_node`)
- [ ] Train the model with new data (blurry ball while dribbling, ball in hands)
- [ ] Estimate player distance
- [ ] Motor control subscriber node
- [ ] ROS 2 launch file

## Custom Model

The ball detection model was trained using the Ultralytics platform.

## License

MIT
