# Tetris Console

A simple, console-based Tetris game written in C++ for Linux. Based on [a YouTube video by javidx9](https://www.youtube.com/watch?v=8OK8_tHeCIA). It uses standard terminal escapes and POSIX functions to manage input and display.

## Overview

The game features:
- Standard Tetris shapes (Tetrominoes).
- Movement and rotation controls.
- Randomly generated pieces.
- Score tracking with exponential bonuses for clearing multiple lines.
- Increasing difficulty (game speed increases every 10 pieces).
- Game over detection.

## Requirements

- **Operating System:** Linux (uses POSIX `termios.h` and `unistd.h`).
- **Compiler:** C++26 compatible compiler (e.g., GCC 14+ or Clang 18+).
- **Build System:** CMake 4.1 or higher.

## Setup and Build

1. **Clone the repository:**
   ```bash
   git clone https://github.com/klaben-szabolcs-bence/tetris-console.git
   cd tetris-console
   ```

2. **Generate build files and build the project:**
   Using the default build system:
   ```bash
   mkdir build && cd build
   cmake ..
   cmake --build .
   ```

## Run Commands

Once built, you can run the executable directly:

```bash
./tetris_console
```

Alternatively, if you are using CLion or another CMake-integrated IDE, you can run the `tetris_console` target directly.

## Controls

The game is controlled via the keyboard:

- `W`: Rotate piece clockwise.
- `A`: Move piece left.
- `S`: Move piece down (fast).
- `D`: Move piece right.

The game ends when a new piece cannot be placed on the field. After a "GAME OVER", press `Enter` to exit.

## Project Structure

For such a simple game, I decided to keep the project structure simple and straightforward.

- `main.cpp`: Contains the entire game logic, including input handling, game loop, rendering, and collision detection.
- `CMakeLists.txt`: CMake configuration for the project.

## Scripts and Development

- **Build Target:** `tetris_console` (executable).
- **Standard:** C++26.
