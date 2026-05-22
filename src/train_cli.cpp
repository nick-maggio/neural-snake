#include "population.h"
#include "game_state.h"
#include "neural_agent.h"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <vector>
#include <windows.h>

static void watchAgentPlay(const std::vector<double>& genome,
                           int boardW, int boardH, uint32_t gameSeed) {
  GameState   game(boardW, boardH, gameSeed);
  NeuralAgent agent(0);
  agent.getNet().setGenome(genome);

  // Hide cursor and clear screen for the initial frame.
  std::cout << "\033[2J\033[H\033[?25l";
  while (game.isAlive()) {
    std::cout << "\033[H";
    game.printGame();
    game.step(agent.chooseAction(game));
    Sleep(50);
  }

  std::cout << "\033[2J\033[H";
  game.printGame();
  std::cout << "\nGame over. Final score: " << game.getScore() << '\n';
  std::cout << "\033[?25h";
}

int main() {
  uint32_t seed = static_cast<uint32_t>(
    std::chrono::high_resolution_clock::now().time_since_epoch().count());
  std::cout << "Seed: " << seed << '\n';

  const int popSize     = 60;
  const int generations = 1300;
  const int boardW      = 10;
  const int boardH      = 10;

  Population pop(popSize, boardW, boardH, 11, 16, 3, seed);

  std::cout << std::fixed << std::setprecision(2);
  std::cout << "Gen | BestFit  | AvgFit  | BestScore | AvgScore\n";
  std::cout << "----+----------+---------+-----------+---------\n";

  for (int gen = 0; gen < generations; ++gen) {
    GenerationStats stats = pop.runGeneration();
    std::cout << std::setw(3) << gen               << " | "
              << std::setw(8) << stats.bestFitness  << " | "
              << std::setw(7) << stats.avgFitness   << " | "
              << std::setw(9) << stats.bestScore    << " | "
              << std::setw(7) << stats.avgScore     << '\n';
  }

  std::cout << "\nWatching best agent play...\n";
  Sleep(1000);
  watchAgentPlay(pop.bestGenome(), boardW, boardH, seed);
}
