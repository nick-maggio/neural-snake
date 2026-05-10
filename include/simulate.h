#pragma once

#include "game_state.h"
#include "agent.h"

struct SimulationResult {
  int    score;
  int    steps;
  double fitness;
};

SimulationResult simulate(GameState& game, Agent& agent);
