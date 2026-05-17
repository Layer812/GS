# GRAVITY SPIN 🌀
### A High-Performance IMU-Driven Physics Maze Game for M5Stack AtomS3R

[![PlatformIO](https://img.shields.io/badge/PlatformIO-Supported-orange.svg)](https://platformio.org/)
[![Hardware](https://img.shields.io/badge/Hardware-M5Stack%20AtomS3R-blue.svg)](https://docs.m5stack.com/en/core/AtomS3R)
[![Framework](https://img.shields.io/badge/Framework-Arduino%20/%20M5Unified-green.svg)](https://github.com/m5stack/M5Unified)

**GRAVITY SPIN** (also known as *THE TILT MAZE*) is a premium, action-packed retro physics game developed for the M5Stack AtomS3R. By utilizing the built-in IMU, players physically tilt their device in any direction to rotate the maze, harnessing gravity to guide a neon sphere to the goal.
<img width="1536" height="2048" alt="Image" src="https://github.com/user-attachments/assets/010416e8-7780-4ad3-b6b9-1fc95b55dc05" />
---

## ⚡ Quick Start: Flash Instantly!

You don't need to compile the code to play! You can flash the pre-compiled binary directly to your M5Stack AtomS3R using **M5Burner**.

1. Download and open [M5Burner](https://docs.m5stack.com/en/download).
2. Connect your **M5Stack AtomS3R** to your computer.
3. Search for the project or use the **Share Code** below:

```text
M5Burner Share Code: AaSaeaFOkPyvucUL
```

4. Click **Burn** and start tilting!

---

## 🎮 Gameplay & Mechanics

*   **Continuous 360° IMU Tilt**: Unlike naive angle-to-force systems that break or glitch at the -180° / 180° boundary, GRAVITY SPIN uses an **advanced vector low-pass filter** directly on raw accelerometer values. This delivers flawless 360-degree rotation without sudden jumps or visual shudder.
*   **16-Point Radial Collision**: The sphere features realistic, elastic collision physics against the walls. By sampling 16 contact points around the perimeter, the ball bounces naturally and slides smoothly along walls.
*   **Premium Retro Visuals**: Built with custom 16-bit scanlines, neon gold/neon red color schemes, a beautiful 3D shaded ball with simulated glass reflections, and smooth stage-clear overlays.

---

## 🗺️ Stages Overview

GRAVITY SPIN features **5 challenging levels** designed to test your dexterity and speed:

1.  **Stage 1 (Beginner)**: A gentle introduction to get accustomed to the tilt response.
2.  **Stage 2 (Spiral)**: A winding vortex path requiring precise, slow-rolling control.
3.  **Stage 3 (Zigzag)**: Sharp corners that test your quick counter-tilting reflexes.
4.  **Stage 4 (Center Goal)**: A minimalistic maze with only outer walls. Guide the ball directly into the central goal using momentum!
5.  **Stage 5 (Hardcore)**: A complex, tight-corridor labyrinth that demands absolute mastery of physics.

---

## 🛠️ Hardware & Libraries

This project is built from scratch utilizing the high-performance M5Unified and M5GFX ecosystem:

*   **Microcontroller**: [M5Stack AtomS3R](https://docs.m5stack.com/en/core/AtomS3R) (featuring high-speed PSRAM & vibrant LCD).
*   **IMU**: On-board BMI270 accelerometer/gyroscope.
*   **Graphics Engine**: **M5GFX** (using a `LGFX_Sprite` offscreen double-buffering architecture for zero screen flicker and buttery-smooth 60 FPS rendering).
*   **System Core**: **M5Unified** for robust, cross-platform IMU initialization and button input polling.

---

## 🚀 Building from Source (PlatformIO)

If you wish to modify or build the project yourself, follow these steps:

### Prerequisites
*   Install [VS Code](https://code.visualstudio.com/) and the [PlatformIO IDE](https://platformio.org/install/ide/vscode) extension.

### Compilation
1.  Clone this repository.
2.  Open the project folder in VS Code (PlatformIO will automatically detect the configuration).
3.  Connect your AtomS3R via USB-C.
4.  Click the **PlatformIO: Build** button (check icon) and then **PlatformIO: Upload** (arrow icon).

### Key Game Controls
*   **Tilt Device**: Rotates the maze and shifts gravity.
*   **Press AtomS3R Screen (BtnA)**:
    *   **Title Screen**: Start Game.
    *   **Stage Clear**: Advance to Next Stage.
    *   **Stage Fail / Game Over**: Return to Title.
    *   **During Gameplay**: Instantly abort and return to Title.

---

## 📜 License
This project is open-source. Feel free to modify, distribute, and build your own custom stages!
****
