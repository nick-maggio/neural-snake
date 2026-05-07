#include "game_state.h"
#include "random_agent.h"
#include "simulate.h"
#include <chrono>
#include <iostream>

int main() {
  uint32_t seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
  std::cout << "Seed: " << seed << '\n';

  int num_games = 100;
  double total_fitness = 0;
  int total_score = 0;
  int max_score = 0;

  for (int i = 0; i < num_games; ++i) {
    GameState game(10, 10, seed + i);
    RandomAgent agent(seed + i + 10000);
    SimulationResult result = simulate(game, agent);
    total_fitness += result.fitness;
    total_score += result.score;
    if (result.score > max_score) max_score = result.score;
  }

  std::cout << "Games: " << num_games << '\n';
  std::cout << "Average score: " << (double)total_score / num_games << '\n';
  std::cout << "Max score: " << max_score << '\n';
  std::cout << "Average fitness: " << total_fitness / num_games << '\n';
}