#include "neural_agent.h"
#include "encode_state.h"
#include <vector>

NeuralAgent::NeuralAgent(uint32_t seed) : net(11, 16, 3, seed) {}

Action NeuralAgent::chooseAction(const GameState& game) {
  std::vector<double> features = encodeState(game);
  std::vector<double> output   = net.forward(features);

  int best = 0;
  for (int i = 1; i < static_cast<int>(output.size()); ++i) {
    if (output[i] > output[best]) best = i;
  }

  Action actions[] = {Action::GoStraight, Action::TurnRight, Action::TurnLeft};
  return actions[best];
}
