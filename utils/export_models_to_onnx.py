#!/usr/bin/env python3
"""
Script pour exporter les modèles YOLO (.pt) vers le format ONNX (.onnx)
nécessaire pour la version C++ du détecteur.

Usage: python export_models_to_onnx.py
"""

from ultralytics import YOLO
from configparser import ConfigParser
import os

def export_model(pt_path: str) -> str:
    """Exporte un modèle YOLO .pt vers ONNX et retourne le chemin du fichier ONNX."""
    if not os.path.exists(pt_path):
        print(f"Erreur: Modèle non trouvé: {pt_path}")
        return None

    print(f"Export de {pt_path} vers ONNX...")
    model = YOLO(pt_path)

    # Export vers ONNX avec les paramètres optimaux pour OpenCV DNN
    onnx_path = model.export(
        format='onnx',
        imgsz=640,
        simplify=True,
        opset=12
    )

    print(f"  -> Exporté: {onnx_path}")
    return onnx_path


if __name__ == "__main__":
    config = ConfigParser()
    config.read("config/config.ini")

    person_model_path = config.get("default", "PERSON_MODEL_PATH")
    basket_model_path = config.get("default", "BASKET_MODEL_PATH")

    print("=== Export des modèles YOLO vers ONNX ===\n")

    person_onnx = export_model(person_model_path)
    basket_onnx = export_model(basket_model_path)

    print("\n=== Export terminé ===")
    print("\nMettez à jour config/config.ini avec les chemins ONNX:")
    if person_onnx:
        print(f"  PERSON_MODEL_PATH = {person_onnx}")
    if basket_onnx:
        print(f"  BASKET_MODEL_PATH = {basket_onnx}")
