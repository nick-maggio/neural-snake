#pragma once

#include "agent.h"
#include "neural_net.h"

class NeuralAgent : public Agent {
  NeuralNet net;

public:
  NeuralAgent(uint32_t seed);
  Action chooseAction(const GameState& game) override;

  NeuralNet&       getNet()       { return net; }
  const NeuralNet& getNet() const { return net; }
};
