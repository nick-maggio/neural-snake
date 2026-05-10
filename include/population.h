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

  std::mt19937 rng;

  double eliteFraction    = 0.25;
  double mutationRate     = 0.20;
  double mutationStrength = 0.3;

public:
  Population(int popSize, int boardW, int boardH,
             int inputSize, int hiddenSize, int outputSize,
             uint32_t seed);

  GenerationStats runGeneration();

  const std::vector<double>& bestGenome() const;
};
