#include "simulate.h"

static constexpr int MAX_STEPS = 2000;

SimulationResult simulate(GameState& game, Agent& agent) {
  int steps = 0;

  while (game.isAlive() && steps < MAX_STEPS) {
    Action a = agent.chooseAction(game);
    game.step(a);
    ++steps;
  }

  int score = game.getScore();
  double fitness = static_cast<double>(score) * score * 1000.0 - (steps * 2);
  return {score, steps, fitness};
}
