#include "game_state.h"
#include <chrono>
#include <iostream>
#include <random>

int main() {
  uint32_t seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
  std::cout << "Seed: " << seed << '\n';
  GameState game(10, 10, seed);
  game.printGame();

  std::mt19937 action_rng(seed);
  std::uniform_int_distribution<int> action_dist(0, 2);
  Action actions[] = {Action::GoStraight, Action::TurnLeft, Action::TurnRight};

  while (game.isAlive()) {
    Action a = actions[action_dist(action_rng)];
    game.step(a);
  }

  std::cout << "\nFinal state:\n";
  game.printGame();
  std::cout << "Snake length: " << game.getSizeOfSnake() << '\n';
}