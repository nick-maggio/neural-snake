#pragma once

#include "game_state.h"

class Agent{
public:
  virtual ~Agent() = default;
  virtual Action chooseAction(const GameState& game) = 0;
};