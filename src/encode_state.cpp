#include "encode_state.h"

static Direction rotate(Direction d, Action turn) {
  if (turn == Action::GoStraight) return d;
  if (turn == Action::TurnLeft) {
    switch (d) {
      case Direction::Up:    return Direction::Left;
      case Direction::Down:  return Direction::Right;
      case Direction::Left:  return Direction::Down;
      case Direction::Right: return Direction::Up;
    }
  } else {
    switch (d) {
      case Direction::Up:    return Direction::Right;
      case Direction::Down:  return Direction::Left;
      case Direction::Left:  return Direction::Up;
      case Direction::Right: return Direction::Down;
    }
  }
  return d;
}

static Point nextCell(Point head, Direction d) {
  switch (d) {
    case Direction::Up:    return {head.x,     head.y - 1};
    case Direction::Down:  return {head.x,     head.y + 1};
    case Direction::Left:  return {head.x - 1, head.y    };
    case Direction::Right: return {head.x + 1, head.y    };
  }
  return head;
}

static bool isDanger(Point cell, const GameState& game) {
  if (cell.x < 0 || cell.x >= game.getWidth() ||
      cell.y < 0 || cell.y >= game.getHeight()) {
    return true;
  }
  for (const auto& seg : game.getSnake()) {
    if (seg.x == cell.x && seg.y == cell.y) return true;
  }
  return false;
}

// Returns an 11-element feature vector:
//   [0-2]  danger ahead / left / right (relative to current heading)
//   [3-6]  one-hot current direction (Up, Down, Left, Right)
//   [7-10] food left / right / above / below (screen coords, y increases downward)
std::vector<double> encodeState(const GameState& game) {
  std::vector<double> f(11, 0.0);

  Point     head = game.getSnake().front();
  Direction dir  = game.getDirection();
  Point     food = game.getFood();

  f[0] = isDanger(nextCell(head, rotate(dir, Action::GoStraight)), game) ? 1.0 : 0.0;
  f[1] = isDanger(nextCell(head, rotate(dir, Action::TurnLeft)),   game) ? 1.0 : 0.0;
  f[2] = isDanger(nextCell(head, rotate(dir, Action::TurnRight)),  game) ? 1.0 : 0.0;

  f[3] = (dir == Direction::Up)    ? 1.0 : 0.0;
  f[4] = (dir == Direction::Down)  ? 1.0 : 0.0;
  f[5] = (dir == Direction::Left)  ? 1.0 : 0.0;
  f[6] = (dir == Direction::Right) ? 1.0 : 0.0;

  f[7]  = (food.x < head.x) ? 1.0 : 0.0;
  f[8]  = (food.x > head.x) ? 1.0 : 0.0;
  f[9]  = (food.y < head.y) ? 1.0 : 0.0;
  f[10] = (food.y > head.y) ? 1.0 : 0.0;

  return f;
}
