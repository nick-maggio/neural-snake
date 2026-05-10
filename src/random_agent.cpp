#include "random_agent.h"

Action RandomAgent::chooseAction(const GameState& /*game*/) {
  Action actions[] = {Action::GoStraight, Action::TurnLeft, Action::TurnRight};
  return actions[dist(rng)];
}
