#pragma once

#include "game_state.h"
#include "neural_agent.h"

#include <cstdint>
#include <memory>
#include <vector>

class Showcase {
public:
  struct Slot {
    std::unique_ptr<GameState>   game;
    std::unique_ptr<NeuralAgent> agent;
  };

  Showcase() = default;

  void reset(int boardW, int boardH,
             const std::vector<std::vector<double>>& genomes,
             const std::vector<uint32_t>&            seeds)
  {
    slots.clear();
    slots.reserve(genomes.size());
    for (size_t i = 0; i < genomes.size(); ++i) {
      Slot s;
      s.game  = std::make_unique<GameState>(boardW, boardH, seeds[i]);
      s.agent = std::make_unique<NeuralAgent>(seeds[i] ^ 0xA5A5A5A5u);
      s.agent->getNet().setGenome(genomes[i]);
      slots.push_back(std::move(s));
    }
  }

  void step() {
    for (auto& s : slots) {
      if (s.game->isAlive()) {
        Action a = s.agent->chooseAction(*s.game);
        s.game->step(a);
      }
    }
  }

  bool allDead() const {
    for (const auto& s : slots) {
      if (s.game->isAlive()) return false;
    }
    return true;
  }

  int              size()      const { return (int)slots.size(); }
  const GameState& game(int i) const { return *slots[i].game; }

private:
  std::vector<Slot> slots;
};
