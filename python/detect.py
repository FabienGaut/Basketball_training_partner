#!/usr/bin/env python3
import cv2
import time
import logging
from pathlib import Path
from configparser import ConfigParser

import torch
from ultralytics import YOLO

# TODO: Quantification INT8 — exporter avec model.export(format='onnx', int8=True)
# TODO: TensorRT (si GPU NVIDIA) — model.export(format='engine') pour gain 3-5x
# TODO: OpenVINO (si Intel CPU) — model.export(format='openvino') pour gain 2-3x
# TODO: NCNN (si ARM/Raspberry Pi) — model.export(format='ncnn')
# TODO: Fusionner les 2 modèles en un seul multi-classe (person + ball) pour diviser l'inférence par 2
# TODO: Découpler capture webcam dans un thread séparé (producteur/consommateur)


def point_in_box(px, py, box):
    x1, y1, x2, y2 = box
    return x1 <= px <= x2 and y1 <= py <= y2


def capture(config, person_model, basket_model):
    conf_threshold = config.getfloat("default", "confidence_threshold")
    webcam_index = config.getint("default", "webcam_index")
    frame_width = config.getint("default", "FRAME_WIDTH")
    frame_height = config.getint("default", "FRAME_HEIGHT")
    process_every_n = config.getint("default", "process_every_n_frames", fallback=1)
    input_size = 320  # Reduced from 640 for ~3x faster inference

    cap = cv2.VideoCapture(webcam_index)
    cap.set(cv2.CAP_PROP_BUFFERSIZE, 1)
    cap.set(cv2.CAP_PROP_FRAME_WIDTH, frame_width)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, frame_height)

    if not cap.isOpened():
        raise RuntimeError("Webcam not accessible")

    device = next(person_model.parameters()).device
    use_half = device.type == "cuda"

    prev_time = time.time()
    fps = 0.0
    frame_count = 0
    fps_update_interval = 10

    # Skip-frame: reuse last detections on skipped frames
    last_persons = []
    last_balls = []
    skip_counter = 0

    while True:
        ret, frame = cap.read()
        if not ret:
            break

        frame_count += 1
        if frame_count >= fps_update_interval:
            current_time = time.time()
            elapsed = current_time - prev_time
            fps = frame_count / elapsed if elapsed > 0 else 0
            prev_time = current_time
            frame_count = 0

        # Inference with skip-frame
        skip_counter += 1
        if skip_counter >= process_every_n:
            skip_counter = 0

            person_results = person_model(
                frame,
                conf=conf_threshold,
                classes=[0],  # COCO: person = 0
                verbose=False,
                imgsz=input_size,
                half=use_half,
            )[0]

            basket_results = basket_model(
                frame,
                conf=conf_threshold,
                verbose=False,
                imgsz=input_size,
                half=use_half,
            )[0]

            last_persons = []
            last_balls = []

            for box in person_results.boxes:
                x1, y1, x2, y2 = map(int, box.xyxy[0])
                conf = float(box.conf[0])
                last_persons.append((x1, y1, x2, y2, conf))

            for box in basket_results.boxes:
                cls_id = int(box.cls[0])
                cls_name = basket_model.names[cls_id]

                if cls_name in ["basketball", "sports ball"]:
                    x1, y1, x2, y2 = map(int, box.xyxy[0])
                    conf = float(box.conf[0])
                    last_balls.append((x1, y1, x2, y2, conf))

        persons = last_persons
        balls = last_balls

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
    config_path = Path(__file__).parent.parent / "config" / "config.ini"
    config = ConfigParser()
    config.read(config_path)

    log_level = config.get("default", "log_level", fallback="INFO").upper()
    logging.basicConfig(
        level=getattr(logging, log_level),
        format="%(asctime)s - %(levelname)s - %(message)s"
    )
    logger = logging.getLogger(__name__)

    person_model_path = config.get("default", "PERSON_MODEL_PATH")
    basket_model_path = config.get("default", "BASKET_MODEL_PATH")

    device = "cuda" if torch.cuda.is_available() else "cpu"
    logger.info(f"Device: {device}")

    logger.info("Loading person model...")
    person_model = YOLO(person_model_path).to(device)

    logger.info("Loading basket model...")
    basket_model = YOLO(basket_model_path).to(device)

    logger.debug(f"Person model classes: {person_model.names}")
    logger.debug(f"Basket model classes: {basket_model.names}")

    capture(config, person_model, basket_model)


if __name__ == "__main__":
    main()
