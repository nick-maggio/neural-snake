#include "simulate.h"
#include <cstdlib>

static constexpr int MAX_STEPS = 2000;

SimulationResult simulate(GameState& game, Agent& agent) {
  int steps        = 0;
  int closerSteps  = 0;
  int fartherSteps = 0;

  while (game.isAlive() && steps < MAX_STEPS) {
    Point headBefore = game.getHead();
    Point food       = game.getFood();
    int distBefore   = std::abs(headBefore.x - food.x) + std::abs(headBefore.y - food.y);

    Action a = agent.chooseAction(game);
    game.step(a);
    ++steps;

    if (game.isAlive()) {
      Point headAfter = game.getHead();
      Point foodAfter = game.getFood();
      int distAfter   = std::abs(headAfter.x - foodAfter.x) + std::abs(headAfter.y - foodAfter.y);

      if (distAfter < distBefore)      ++closerSteps;
      else if (distAfter > distBefore) ++fartherSteps;
    }
  }

  int score = game.getScore();
  // score² dominates to reward food collection; steps and directional progress
  // break ties between similarly-scoring genomes.
  double fitness = static_cast<double>(score) * score * 1000.0
                 + steps
                 + closerSteps  * 0.5
                 - fartherSteps * 0.3;
  return {score, steps, fitness};
}
