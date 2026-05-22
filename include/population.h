#pragma once

#include "neural_agent.h"
#include "simulate.h"
#include <cstdint>
#include <random>
#include <vector>

struct GenerationStats {
  double bestFitness;
  double avgFitness;
  int    bestScore;
  double avgScore;
};

class Population {
  int popSize;
  int inputSize;
  int hiddenSize;
  int outputSize;
  int boardWidth;
  int boardHeight;

  std::vector<std::vector<double>> genomes;
  std::vector<double>              fitnesses;
  // Stored so any game from the last generation can be replayed with the exact same food sequence.
  std::vector<uint32_t>            gameSeeds;

  std::mt19937 rng;

  double eliteFraction    = 0.25;
  double mutationRate     = 0.30;
  double mutationStrength = 0.3;

public:
  Population(int popSize, int boardW, int boardH,
             int inputSize, int hiddenSize, int outputSize,
             uint32_t seed);

  GenerationStats runGeneration();

  const std::vector<double>& bestGenome() const;

  // All accessors below describe the most recent generation. Calling runGeneration() overwrites them.
  std::vector<int>           topNIndices(int n) const; // sorted best-first; clamped to popSize

  // Per-agent data for replay. Index must be < popSize.
  const std::vector<double>& genome(int i)   const { return genomes[i]; }
  uint32_t                   gameSeed(int i) const { return gameSeeds[i]; }
  double                     fitness(int i)  const { return fitnesses[i]; }

  int                        boardW()        const { return boardWidth; }
  int                        boardH()        const { return boardHeight; }
};