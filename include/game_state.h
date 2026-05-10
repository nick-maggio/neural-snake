#pragma once

#include <cstdint>
#include <deque>
#include <random>

enum class Direction { Up, Down, Left, Right };
enum class Action    { GoStraight, TurnRight, TurnLeft };

struct Point {
  int x;
  int y;
};

class GameState {
  int width;
  int height;
  std::deque<Point> snake;
  Direction dir;
  Point food;
  int score          = 0;
  bool alive         = true;
  int stepsSinceFood = 0;
  std::mt19937 rng;

  Point calculateNewHead();
  bool  headTouchesWall(Point head);
  bool  headTouchesBody(Point head);
  void  spawnFood();
  void  setDirection(Action action);

public:
  GameState(int w, int h, uint32_t seed);

  void step(Action action);
  void printGame();

  int   getScore()       const;
  bool  isAlive()        const;
  int   getSizeOfSnake() const;
  int   getWidth()       const { return width; }
  int   getHeight()      const { return height; }
  Point getHead()        const { return snake.front(); }
  Point getFood()        const { return food; }
  Direction                getDirection() const { return dir; }
  const std::deque<Point>& getSnake()     const { return snake; }
};
