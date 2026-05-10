# neural-snake

A genetic algorithm that trains a small neural network to play Snake. Agents are evaluated by fitness score, the top 25% (% adjustable in include/population.h) of performers are kept as elites, and the next generation is based on mutated copies of those elites.

## Structure

| Component | Description |
|-----------|-------------|
| `GameState` | Core Snake engine features: board, movement, collision, food |
| `NeuralNet` | Two-layer fully-connected network (ReLU hidden, linear output) |
| `NeuralAgent` | Wraps `NeuralNet` into an agent; converts output from network to an action |
| `encodeState` | Returns an 11-feature vector (danger, food direction, etc.) |
| `Population` | Genetic algorithm loop: evaluate, rank, select elites, mutate offspring |
| `simulate` | Runs one game to completion and returns a fitness score |
| `RandomAgent` | Random baseline agent |

### Neural Network

```
Input (11)  →  Hidden (16, ReLU)  →  Output (3, linear)
```

Output neurons correspond to `GoStraight`, `TurnRight`, `TurnLeft`. The action with the highest activation is chosen.

### Fitness function

```
fitness = score² × 1000 + steps + closerSteps × 0.5 − fartherSteps × 0.3
```

The quadratic score term strongly rewards food collection; step count and directional progress break ties between genomes to encourage efficiency, rather than inefficient safety (Scanning the board line by line to guarantee winning: correct, but not fun).

### GA hyperparameters // All adjustable

| Parameter | Value |
|-----------|-------|
| Population size | 150 |                 customize in src/main.cpp
| Generations | 200 |                     customize in src/main.cpp
| Board size | 10 × 10 |                  customize in src/main.cpp
| Elite fraction | 25% |                  customize in include/population.h
| Mutation rate (per weight) | 20% |      customize in include/population.h
| Mutation strength | 0.3 |           customize in include/population.h

## System requirements

| Requirement | Details |
|-------------|---------|
| OS | Windows (uses `Sleep` from `<windows.h>` and ANSI terminal escape codes) |
| Compiler | g++ with C++17 support (`-std=c++17`) |
| Build tool | GNU Make |
| Terminal | Any terminal with ANSI escape code support (Windows Terminal, ConEmu, VS Code terminal) |

## Build

```bash
make
```

Some users may need to use:

```bash
mingW32-make
```

Produces `snake.exe` in the project root.

```bash
make clean   # remove the binary
```

## Run

```bash
./snake.exe
```

The program trains for 200 generations by default (this will likely need to be reduced along with population size on slower systems), printing a fitness/score table, then plays back the best agent in the terminal.
