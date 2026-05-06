#include "game_state.h"
#include <iostream>
#include <vector>

GameState::GameState(int w, int h, uint32_t seed)
  : width(w), height(h), dir(Direction::Right), rng(seed) {
  snake.push_back({w / 2, h / 2});
  spawnFood();
}

Point GameState::calculateNewHead() {
  Point head = snake.front();
  Point newHead = head;

  switch (dir) {
    case Direction::Up:    newHead.y -= 1; break;
    case Direction::Down:  newHead.y += 1; break;
    case Direction::Left:  newHead.x -= 1; break;
    case Direction::Right: newHead.x += 1; break;
  }

  return newHead;
}

bool GameState::headTouchesWall(Point head) {
  if (head.x < 0 || head.x >= width ||
      head.y < 0 || head.y >= height) {
    alive = false;
    return true;
  }
  return false;
}

bool GameState::headTouchesBody(Point head) {
  bool willGrow = (head.x == food.x && head.y == food.y);
  auto end = willGrow ? snake.end() : std::prev(snake.end());
  for (auto it = snake.begin(); it != end; ++it) {
    if (it->x == head.x && it->y == head.y) {
      alive = false;
      return true;
    }
  }
  return false;
}

void GameState::spawnFood() {
  std::uniform_int_distribution<int> dx(0, width - 1);
  std::uniform_int_distribution<int> dy(0, height - 1);
  while (true) {
    Point p{dx(rng), dy(rng)};
    bool onSnake = false;
    for (const auto& s : snake) {
      if (s.x == p.x && s.y == p.y) { onSnake = true; break; }
    }
    if (!onSnake) { food = p; return; }
  }
}

void GameState::setDirection(Action action) {
  if (action == Action::TurnLeft) {
    switch (dir) {
      case Direction::Up:    dir = Direction::Left;  break;
      case Direction::Down:  dir = Direction::Right; break;
      case Direction::Left:  dir = Direction::Down;  break;
      case Direction::Right: dir = Direction::Up;    break;
    }
  } else if (action == Action::TurnRight) {
    switch (dir) {
      case Direction::Up:    dir = Direction::Right; break;
      case Direction::Down:  dir = Direction::Left;  break;
      case Direction::Left:  dir = Direction::Up;    break;
      case Direction::Right: dir = Direction::Down;  break;
    }
  }
}

void GameState::step(Action action) {
  if (!alive) return;

  setDirection(action);

  Point newHead = calculateNewHead();

  if (headTouchesWall(newHead)) return;
  if (headTouchesBody(newHead)) return;

  snake.push_front(newHead);

  bool willGrow = newHead.x == food.x && newHead.y == food.y;

  if (willGrow) {
    ++score;
    stepsSinceFood = 0;
    spawnFood();
  } else {
    snake.pop_back();
    ++stepsSinceFood;
  }

  if (stepsSinceFood > 100 * width) {
    alive = false;
  }
}

int GameState::getScore() const { return score; }
bool GameState::isAlive() const { return alive; }
int GameState::getSizeOfSnake() const { return snake.size(); }

void GameState::printGame() {
  std::vector<std::vector<char>> grid(height, std::vector<char>(width, '.'));

  grid[food.y][food.x] = '*';

  for (auto it = snake.begin(); it != snake.end(); ++it) {
    grid[it->y][it->x] = 'o';
  }

  grid[snake.front().y][snake.front().x] = '@';

  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      std::cout << grid[y][x];
    }
    std::cout << '\n';
  }

  std::cout << "Score: " << score << (alive ? "" : " (dead)") << '\n';
}