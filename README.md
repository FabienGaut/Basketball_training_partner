## 🚧 Project status: Work in progress

# Basketball Player Detection - Robot Defender

Système de vision par ordinateur pour robot défenseur de basketball. Le projet détecte en temps réel les joueurs tenant un ballon et transmet les coordonnées du centre de leur bounding box via ROS pour permettre au robot de se positionner comme un défenseur face au porteur du ballon.

## Objectif

Le robot doit :
1. **Détecter** quand une personne tient un ballon de basket (classification "basketball player")
2. **Récupérer** les coordonnées du centre de la bounding box du joueur
3. **Transmettre** ces coordonnées via ROS aux composants du robot (servomoteurs, moteurs)
4. **Se positionner** comme un défenseur en restant face au porteur du ballon à distance appropriée

## Principe de détection

Le système utilise deux modèles YOLO en parallèle :

| Modèle | Fichier | Rôle |
|--------|---------|------|
| **COCO** | `yolo26n` | Détection des personnes (classe 0) |
| **Custom** | `ballDetection` | Détection des ballons de basket |

**Règle de classification** : Une personne est identifiée comme **"basketball player"** si le centre d'un ballon détecté se trouve à l'intérieur de sa bounding box.

## Versions disponibles

Le projet propose trois implémentations :

| Version | Performance | Format modèles | Cas d'usage |
|---------|-------------|----------------|-------------|
| **ROS 2** | 5-10 FPS | ONNX | Intégration robot (recommandée) |
| **C++ standalone** | 5-10.4 FPS | ONNX | Tests sans ROS |
| **Python** | 4-9.4 FPS | PyTorch | Prototypage |

La version ROS 2 est recommandée pour l'intégration avec le robot défenseur.

## Structure du projet

```
ball_detection/
├── config/
│   └── config.ini              # Configuration partagée
├── cpp/                        # Version C++ standalone
│   ├── CMakeLists.txt
│   ├── include/
│   │   ├── Config.hpp
│   │   ├── YOLODetector.hpp
│   │   ├── Capture.hpp
│   │   ├── Detection.hpp
│   │   └── Utils.hpp
│   ├── src/
│   │   ├── main.cpp
│   │   ├── Config.cpp
│   │   ├── YOLODetector.cpp
│   │   ├── Capture.cpp
│   │   └── Utils.cpp
│   └── build/
├── ros2_ws/                    # Workspace ROS 2
│   └── src/
│       └── basketball_detection/
│           ├── CMakeLists.txt
│           ├── package.xml
│           ├── include/basketball_detection/
│           │   ├── Config.hpp
│           │   ├── YOLODetector.hpp
│           │   ├── Capture.hpp
│           │   ├── Detection.hpp
│           │   └── Utils.hpp
│           ├── src/
│           │   ├── publisher_node.cpp  # Noeud ROS principal
│           │   ├── Config.cpp
│           │   ├── YOLODetector.cpp
│           │   ├── Capture.cpp
│           │   └── Utils.cpp
│           └── deps/
│               └── onnxruntime-linux-x64-1.17.0/
├── python/                     # Version Python
│   ├── detect.py
│   └── requirements.txt
├── models/
│   ├── yolo26n.pt              # Modèle COCO (PyTorch)
│   ├── yolo26n.onnx            # Modèle COCO (ONNX)
│   ├── ballDetection.pt        # Modèle custom (PyTorch)
│   └── ballDetection.onnx      # Modèle custom (ONNX)
├── deps/
│   └── onnxruntime-linux-x64-1.17.0/
├── docker/
│   ├── dockerfile_python
│   ├── dockerfile_cpp
│   └── compose.yaml
├── utils/
│   └── export_models_to_onnx.py
├── tests/
├── .gitignore
└── README.md
```

---

## Installation - Version C++

### Prérequis

- CMake 3.10+
- Compilateur C++17 (GCC ou Clang)
- OpenCV 4.x (avec headers de développement)
- Webcam

### Dépendances

ONNX Runtime est inclus dans le dossier `deps/`. Si besoin de le télécharger :

```bash
cd deps/
wget https://github.com/microsoft/onnxruntime/releases/download/v1.17.0/onnxruntime-linux-x64-1.17.0.tgz
tar -xzf onnxruntime-linux-x64-1.17.0.tgz
rm onnxruntime-linux-x64-1.17.0.tgz
```

### Installation OpenCV (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install libopencv-dev
```

### Compilation

```bash
cd cpp
mkdir -p build
cd build
cmake ..
make
```

### Exécution

```bash
./cpp/build/detectBasketBallPlayer
```

Appuyez sur `q` pour quitter.

---

## Installation - Version Python

### Prérequis

- Python 3.8+
- Webcam
- GPU CUDA (optionnel mais recommandé)

### Installation des dépendances

```bash
pip install -r python/requirements.txt
```

Ou manuellement :

```bash
pip install opencv-python>=4.8.0 torch>=2.0.0 ultralytics>=8.0.0
```

### Exécution

```bash
python python/detect.py
```

Appuyez sur `q` pour quitter.

---

## Configuration

Fichier : `config/config.ini`

```ini
[default]
log_level = DEBUG
webcam_index = 2
confidence_threshold = 0.7
PERSON_MODEL_PATH = models/yolo26n.pt      # Python: .pt / C++: .onnx
BASKET_MODEL_PATH = models/ballDetection.pt
FRAME_WIDTH = 640
FRAME_HEIGHT = 480

[Visualisation]
draw_balls = True
draw_players = True
```

| Paramètre | Description | Valeur par défaut |
|-----------|-------------|-------------------|
| `webcam_index` | Index de la webcam | `2` |
| `confidence_threshold` | Seuil de confiance YOLO | `0.7` |
| `FRAME_WIDTH` | Largeur de la frame | `640` |
| `FRAME_HEIGHT` | Hauteur de la frame | `480` |
| `draw_balls` | Afficher les ballons | `True` |
| `draw_players` | Afficher les joueurs | `True` |

## Conversion des modèles

Pour convertir les modèles PyTorch vers ONNX (requis pour la version C++) :

```bash
python utils/export_models_to_onnx.py
```

## Affichage visuel

| Couleur | Signification |
|---------|---------------|
| **Bleu** | Personne sans ballon |
| **Vert** | Basketball player (personne avec ballon) |
| **Orange** | Ballon de basket détecté |

## Architecture technique

```
┌─────────────┐     ┌─────────────┐
│   Webcam    │────▶│  Capture    │
└─────────────┘     └──────┬──────┘
                           │
                           ▼
              ┌────────────────────────┐
              │   Inférence parallèle  │
              │  ┌──────┐  ┌────────┐  │
              │  │COCO  │  │Custom  │  │
              │  │Model │  │Model   │  │
              │  └──────┘  └────────┘  │
              └────────────┬───────────┘
                           │
                           ▼
              ┌────────────────────────┐
              │ Classification:        │
              │ Point-in-box check     │
              └────────────┬───────────┘
                           │
                           ▼
              ┌────────────────────────┐
              │ Coordonnées centre     │
              │ bounding box           │
              └────────────┬───────────┘
                           │
                           ▼
              ┌────────────────────────┐
              │  ROS Publisher         │
              │  (à implémenter)       │
              └────────────┬───────────┘
                           │
                           ▼
              ┌────────────────────────┐
              │  Servomoteurs/Moteurs  │
              │  Robot défenseur       │
              └────────────────────────┘
```

## Installation - Version ROS 2

La version ROS 2 permet d'intégrer la détection dans un système robotique complet.

### Prérequis

- ROS 2 Humble ou plus récent
- colcon build tools
- OpenCV 4.x
- ONNX Runtime (inclus dans `ros2_ws/src/basketball_detection/deps/`)

### Installation ROS 2 (Ubuntu 22.04)

```bash
# Installation ROS 2 Humble
sudo apt update && sudo apt install -y locales
sudo locale-gen en_US en_US.UTF-8
sudo update-locale LC_ALL=en_US.UTF-8 LANG=en_US.UTF-8
export LANG=en_US.UTF-8

sudo apt install software-properties-common
sudo add-apt-repository universe
sudo apt update && sudo apt install curl -y
sudo curl -sSL https://raw.githubusercontent.com/ros/rosdistro/master/ros.key -o /usr/share/keyrings/ros-archive-keyring.gpg
echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/ros-archive-keyring.gpg] http://packages.ros.org/ros2/ubuntu $(. /etc/os-release && echo $UBUNTU_CODENAME) main" | sudo tee /etc/apt/sources.list.d/ros2.list > /dev/null
sudo apt update
sudo apt install ros-humble-desktop ros-dev-tools
```

### Compilation du workspace

```bash
# Sourcer ROS 2
source /opt/ros/humble/setup.bash

# Compiler le workspace
cd ros2_ws
colcon build --packages-select basketball_detection

# Sourcer le workspace
source install/setup.bash
```

### Exécution du noeud

```bash
# Dans un terminal
source /opt/ros/humble/setup.bash
source ros2_ws/install/setup.bash

# Lancer le noeud de détection
ros2 run basketball_detection publisher_node --ros-args -p config_path:=/chemin/vers/config/config.ini
```

### Topic publié

| Topic | Type | Description |
|-------|------|-------------|
| `/basketball_player` | `geometry_msgs/msg/Point` | Coordonnées (x, y) du centre de la bounding box du joueur avec ballon |

### Écouter le topic

```bash
ros2 topic echo /basketball_player
```

### Architecture du noeud ROS 2

```
┌─────────────────────────────────────────────────────────┐
│                  publisher_node                          │
│  ┌─────────────┐                                        │
│  │   Webcam    │                                        │
│  └──────┬──────┘                                        │
│         │                                               │
│         ▼                                               │
│  ┌─────────────────────────────┐                        │
│  │   YOLODetector (x2)         │                        │
│  │   - personDetector (COCO)   │                        │
│  │   - basketDetector (Custom) │                        │
│  └──────────────┬──────────────┘                        │
│                 │                                       │
│                 ▼                                       │
│  ┌─────────────────────────────┐                        │
│  │   Classification            │                        │
│  │   (point-in-box)            │                        │
│  └──────────────┬──────────────┘                        │
│                 │                                       │
│                 ▼                                       │
│  ┌─────────────────────────────┐    ┌─────────────────┐ │
│  │   ROS Publisher             │───▶│ /basketball_player│
│  │   geometry_msgs/Point       │    │ topic            │ │
│  └─────────────────────────────┘    └─────────────────┘ │
└─────────────────────────────────────────────────────────┘
```

---

## TODO

- [x] Envoi des données sur un topic ROS 2
- [ ] Entrainer le modele avec des nouvelles données pour pouvoir identifier une balle floue en dribble ou dans les mains du joueur
- [ ] Réussir à estimer la distance du joueur
- [ ] Noeud subscriber pour les servomoteurs
- [ ] Noeud subscriber pour les moteurs de déplacement
- [ ] Lancement via launch file ROS 2

## Benchmark

| Version | FPS min | FPS max |
|---------|---------|---------|
| C++ (ONNX) | 5 | 10.4 |
| Python (PyTorch) | 4 | 9.4 |

## Modèle custom

Le modèle de détection de ballon a été entraîné via la plateforme Ultralytics :
[https://platform.ultralytics.com/fabien-gautier/intelligent-osprey/exp](https://platform.ultralytics.com/fabien-gautier/intelligent-osprey/exp?tab=export)

## Licence

Projet académique / personnel.
