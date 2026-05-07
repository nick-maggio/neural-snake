#pragma once

#include "agent.h"
#include <random>

class RandomAgent : public Agent {
  std::mt19937 rng;
  std::uniform_int_distribution<int> dist{0, 2};

public:
  RandomAgent(uint32_t seed) : rng(seed) {}
  Action chooseAction(const GameState& game) override;
};