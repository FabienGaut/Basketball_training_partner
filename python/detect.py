#!/usr/bin/env python3
import cv2
import time
import logging
from pathlib import Path
from configparser import ConfigParser

import torch
from ultralytics import YOLO


def point_in_box(px, py, box):
    """Check if point (px, py) is inside bounding box."""
    x1, y1, x2, y2 = box
    return x1 <= px <= x2 and y1 <= py <= y2


def capture(config, person_model, basket_model):
    """Main capture and detection loop."""
    conf_threshold = config.getfloat("default", "confidence_threshold")
    webcam_index = config.getint("default", "webcam_index")

    cap = cv2.VideoCapture(webcam_index)
    if not cap.isOpened():
        raise RuntimeError("Webcam non accessible")

    # FPS calculation variables
    prev_time = time.time()
    fps = 0.0
    frame_count = 0
    fps_update_interval = 10  # Update FPS every 10 frames

    while True:
        ret, frame = cap.read()
        if not ret:
            break

        # Calculate FPS
        frame_count += 1
        if frame_count >= fps_update_interval:
            current_time = time.time()
            elapsed = current_time - prev_time
            fps = frame_count / elapsed if elapsed > 0 else 0
            prev_time = current_time
            frame_count = 0

        # ========= INFERENCE =========
        person_results = person_model(
            frame,
            conf=conf_threshold,
            classes=[0],  # COCO: person = 0
            verbose=False
        )[0]

        basket_results = basket_model(
            frame,
            conf=conf_threshold,
            verbose=False
        )[0]

        persons = []
        balls = []

        # ========= PERSONS =========
        for box in person_results.boxes:
            x1, y1, x2, y2 = map(int, box.xyxy[0])
            conf = float(box.conf[0])
            persons.append((x1, y1, x2, y2, conf))

        # ========= BALLS =========
        for box in basket_results.boxes:
            cls_id = int(box.cls[0])
            cls_name = basket_model.names[cls_id]

            if cls_name in ["basketball", "sports ball"]:
                x1, y1, x2, y2 = map(int, box.xyxy[0])
                conf = float(box.conf[0])
                balls.append((x1, y1, x2, y2, conf))

        # ========= DRAW BALLS =========
        if config.getboolean("Visualisation", "draw_balls"):
            for bx1, by1, bx2, by2, bconf in balls:
                cx = (bx1 + bx2) // 2
                cy = (by1 + by2) // 2

                cv2.rectangle(frame, (bx1, by1), (bx2, by2), (0, 165, 255), 2)
                cv2.circle(frame, (cx, cy), 4, (0, 165, 255), -1)
                cv2.putText(
                    frame,
                    f"ball {bconf:.2f}",
                    (bx1, by1 - 6),
                    cv2.FONT_HERSHEY_SIMPLEX,
                    0.5,
                    (0, 165, 255),
                    2
                )

        # ========= DRAW PERSONS / PLAYERS =========
        if config.getboolean("Visualisation", "draw_players"):
            for px1, py1, px2, py2, pconf in persons:
                label = "person"
                color = (255, 0, 0)

                for bx1, by1, bx2, by2, _ in balls:
                    cx = (bx1 + bx2) // 2
                    cy = (by1 + by2) // 2

                    if point_in_box(cx, cy, (px1, py1, px2, py2)):
                        label = "basketball player"
                        color = (0, 255, 0)
                        break

                cv2.rectangle(frame, (px1, py1), (px2, py2), color, 2)
                cv2.putText(
                    frame,
                    f"{label} {pconf:.2f}",
                    (px1, py1 - 8),
                    cv2.FONT_HERSHEY_SIMPLEX,
                    0.6,
                    color,
                    2
                )

        # ========= DRAW FPS =========
        cv2.putText(
            frame,
            f"Python | FPS: {fps:.1f}",
            (10, 30),
            cv2.FONT_HERSHEY_SIMPLEX,
            0.8,
            (0, 255, 0),
            2
        )

        cv2.imshow("Basketball Detection (Python)", frame)

        if cv2.waitKey(1) & 0xFF == ord("q"):
            break

    cap.release()
    cv2.destroyAllWindows()


def main():
    # Load config (relative to project root)
    config_path = Path(__file__).parent.parent / "config" / "config.ini"
    config = ConfigParser()
    config.read(config_path)

    # ========= LOGGING =========
    log_level = config.get("default", "log_level", fallback="INFO").upper()
    logging.basicConfig(
        level=getattr(logging, log_level),
        format="%(asctime)s - %(levelname)s - %(message)s"
    )
    logger = logging.getLogger(__name__)

    # ========= CONFIGURATION =========
    person_model_path = config.get("default", "PERSON_MODEL_PATH")
    basket_model_path = config.get("default", "BASKET_MODEL_PATH")

    # ========= LOAD MODELS =========
    device = "cuda" if torch.cuda.is_available() else "cpu"
    logger.info(f"Device: {device}")

    logger.info("Loading person model...")
    person_model = YOLO(person_model_path).to(device)

    logger.info("Loading basket model...")
    basket_model = YOLO(basket_model_path).to(device)

    logger.debug(f"Person model classes: {person_model.names}")
    logger.debug(f"Basket model classes: {basket_model.names}")

    # Start capture
    capture(config, person_model, basket_model)


if __name__ == "__main__":
    main()
