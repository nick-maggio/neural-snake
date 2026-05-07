#include "simulate.h"

SimulationResult simulate(GameState& game, Agent& agent) {
  int steps = 0;
  while (game.isAlive()) {
    Action a = agent.chooseAction(game);
    game.step(a);
    ++steps;
  }
  int score = game.getScore();
  double fitness = score * 1000 + steps;
  return {score, steps, fitness};
}