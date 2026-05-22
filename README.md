# neural-snake

A genetic algorithm that trains a small neural network to play Snake. Agents are evaluated by fitness score; the top 25% survive each generation, and the next generation is built from mutated copies of those elites. Training of the neural network uses multithreading across the CPU for accelerated training speeds. This project also include a from-scratch renderer that uses Win32 and DirectX 11 for custom rendering. As an engaging demonstration of the neural network's training, the top 4 performing agents' gameplay is rendered on the custom graphics system.

<img width="800" height="425" alt="VideoProject1-ezgif com-video-to-gif-converter (1)" src="https://github.com/user-attachments/assets/ed039e1b-a033-4c4d-bec6-afbf627bcf96" />

## Structure

| Component | Description |
|-----------|-------------|
| `GameState` | Core snake engine: board, movement, collision, food |
| `NeuralNet` | Two-layer fully-connected network (ReLU hidden, linear output) |
| `NeuralAgent` | Wraps `NeuralNet`; converts network output to an action |
| `encodeState` | Returns an 11-feature vector (danger, direction, food position) |
| `Population` | Genetic algorithm: evaluate, rank, select elites, mutate offspring |
| `simulate` | Runs one game to completion and returns a fitness score |
| `Showcase` | Holds N parallel games for visual replay of top-N agents |
| `RandomAgent` | Random baseline agent |

`src/train_cli.cpp` is a terminal-only entry point (no rendering) for debug use. As such, it is not included in the default build.

## Renderer

The project includes a custom renderer built from scratch with Win32 and DirectX 11. It creates a lightweight window, draws the top 4 performing agents from the first generation, and then draws the top 4 agents from the final generation in order to visualize the training of the network.

### Neural network

```
Input (11)  →  Hidden (16, ReLU)  →  Output (3, linear)
```

Output neurons direct the snake to `GoStraight`, `TurnRight`, `TurnLeft`. The action with the highest activation is chosen.

### Input features

| Index | Feature |
|-------|---------|
| 0–2 | Danger ahead / left / right (relative to current direction) |
| 3–6 | One-hot current direction (Up, Down, Left, Right) |
| 7–10 | Food left / right / above / below |

### Fitness function

```
fitness = score² × 1000 − steps × 2
```

The quadratic score term strongly rewards food collection; the step penalty discourages the network from scanning the board line-by-line-- It's a safe playing method, but not any fun, so we avoid it. :)

### Genetic Algorithm Parameters

| Parameter | Value | Location |
|-----------|-------|----------|
| Population size | 150 | `src/main.cpp` |
| Generations | 700 | `src/main.cpp` |
| Board size | 10 × 10 | `src/main.cpp` |
| Elite fraction | 25% | `include/population.h` |
| Mutation rate (per weight) | 30% | `include/population.h` |
| Mutation strength | 0.3 | `include/population.h` |

## System requirements

| Requirement | Details |
|-------------|---------|
| OS | Windows (DirectX 11, `<windows.h>`) |
| Compiler | g++ with C++17 support (`-std=c++17`) |
| Build tool | GNU Make |
** Some systems may have trouble during the training of the neural network. Decrease `POP_SIZE` and `GENERATIONS` in `src/main.cpp` if your system has performance issues.

## Build

```bash
make
```

Some users may instead need:

```bash
mingW32-make
```

Produces `snake.exe` in the project root.

## Run

```bash
./snake.exe
```

Opens a DX11 window. Renders the top 4 performing agents from generation 1, trains silently through all remaining generations (printing updates to the console for every 10 generations), then displays the final top 4 performing agents post-training.

Note that while certain design choices were made to limit the agents' tendency to travel in circles continuously, it was impossible to completely remove this behavior without harming the overall training of the neural network. If one of the snakes is circling the board without end, just wait, and it will eventually die.
