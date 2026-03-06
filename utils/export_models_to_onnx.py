#!/usr/bin/env python3
"""
Export YOLO models (.pt) to ONNX format (.onnx).
Usage: python export_models_to_onnx.py
"""

from ultralytics import YOLO
from configparser import ConfigParser
import os

def export_model(pt_path: str) -> str:
    if not os.path.exists(pt_path):
        print(f"Error: Model not found: {pt_path}")
        return None

    print(f"Exporting {pt_path} to ONNX...")
    model = YOLO(pt_path)

    onnx_path = model.export(
        format='onnx',
        imgsz=640,
        simplify=True,
        opset=12
    )

    print(f"  -> Exported: {onnx_path}")
    return onnx_path


if __name__ == "__main__":
    config = ConfigParser()
    config.read("config/config.ini")

    person_model_path = config.get("default", "PERSON_MODEL_PATH")
    basket_model_path = config.get("default", "BASKET_MODEL_PATH")

    print("=== Exporting YOLO models to ONNX ===\n")

    person_onnx = export_model(person_model_path)
    basket_onnx = export_model(basket_model_path)

    print("\n=== Export complete ===")
    print("\nUpdate config/config.ini with the ONNX paths:")
    if person_onnx:
        print(f"  PERSON_MODEL_PATH = {person_onnx}")
    if basket_onnx:
        print(f"  BASKET_MODEL_PATH = {basket_onnx}")
