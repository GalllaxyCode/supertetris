# 🕹️ Neon Tetris

A "juicy", neon-infused Tetris clone written in C using the [Raylib](https://www.raylib.com/) library. 

Instead of just clearing lines, this version features arcade-style visual feedback, screen shake, particle explosions, and random bomb events that blow up your board!

## ✨ Features
* **Juicy Mechanics:** Screen shake on hard drops and massive shockwaves for explosions.
* **Random Events:** A 12% chance to spawn a 3x3 or 6x6 high-explosive bomb instead of a standard piece.
* **Particle Physics:** Cleared lines shatter into individual blocks that obey gravity and velocity.
* **Neon Aesthetics:** Additive blending creates a glowing, arcade-cabinet vibe.
* **Ghost Piece:** A faint projection shows exactly where your piece will land.

## 🎮 Controls
* **Left / Right Arrows:** Move piece
* **Up Arrow:** Rotate piece
* **Down Arrow:** Soft Drop (Speed up falling)
* **Spacebar:** Hard Drop (Instantly lock piece)

## 🛠️ How to Compile and Run

This game requires a C compiler and the **Raylib** library. 

### macOS
1. Install Raylib using Homebrew:
   ```bash
   brew install raylib
