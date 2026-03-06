#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
COMPOSE_FILE="${SCRIPT_DIR}/docker/compose.yaml"
CONTAINER="ball_detection_ros"
CONFIG_PATH="/workspace/ball_detection/config/config.ini"

if ! docker ps --format '{{.Names}}' | grep -q "^${CONTAINER}$"; then
    echo "[INFO] Starting Docker container..."
    docker compose -f "$COMPOSE_FILE" up -d
    sleep 2
else
    echo "[INFO] Container already running."
fi

echo "[INFO] Building ROS 2 workspace..."
docker exec "$CONTAINER" bash -lc "
    source /opt/ros/jazzy/setup.bash &&
    cd /workspace/ball_detection/ros2_ws &&
    colcon build --packages-select basketball_detection controller 2>&1
"

echo "[INFO] Launching publisher_node..."
docker exec -d "$CONTAINER" bash -lc "
    source /opt/ros/jazzy/setup.bash &&
    source /workspace/ball_detection/ros2_ws/install/setup.bash &&
    ros2 run basketball_detection publisher_node \
        --ros-args -p config_path:=${CONFIG_PATH}
"

echo "[INFO] Launching servo_node..."
docker exec -d "$CONTAINER" bash -lc "
    source /opt/ros/jazzy/setup.bash &&
    source /workspace/ball_detection/ros2_ws/install/setup.bash &&
    ros2 run controller servo_node \
        --ros-args -p config_path:=${CONFIG_PATH}
"

echo ""
echo "[INFO] Both nodes are running. Press Ctrl+C to stop."

cleanup() {
    echo ""
    echo "[INFO] Stopping nodes..."
    docker exec "$CONTAINER" bash -lc \
        "pkill -f 'publisher_node' 2>/dev/null; pkill -f 'servo_node' 2>/dev/null; true"
    echo "[INFO] Done."
    exit 0
}

trap cleanup INT TERM

while true; do sleep 1; done
