#include "population.h"
#include "game_state.h"
#include <algorithm>
#include <numeric>
#include <stdexcept>

Population::Population(int popSize, int boardW, int boardH,
                       int inputSize, int hiddenSize, int outputSize,
                       uint32_t seed)
  : popSize(popSize),
    inputSize(inputSize), hiddenSize(hiddenSize), outputSize(outputSize),
    boardWidth(boardW), boardHeight(boardH),
    rng(seed) {
  int gSize = inputSize * hiddenSize + hiddenSize
            + hiddenSize * outputSize + outputSize;

  std::uniform_real_distribution<double> dist(-1.0, 1.0);
  genomes.resize(popSize);
  for (auto& g : genomes) {
    g.resize(gSize);
    for (auto& v : g) v = dist(rng);
  }
  fitnesses.assign(popSize, 0.0);
}

GenerationStats Population::runGeneration() {
  std::vector<int> scores(popSize, 0);

  for (int i = 0; i < popSize; ++i) {
    NeuralAgent agent(rng());
    agent.getNet().setGenome(genomes[i]);
    GameState        game(boardWidth, boardHeight, rng());
    SimulationResult r = simulate(game, agent);
    fitnesses[i] = r.fitness;
    scores[i]    = r.score;
  }

  GenerationStats stats;
  stats.bestFitness = *std::max_element(fitnesses.begin(), fitnesses.end());
  stats.avgFitness  = std::accumulate(fitnesses.begin(), fitnesses.end(), 0.0) / popSize;
  stats.bestScore   = *std::max_element(scores.begin(), scores.end());
  stats.avgScore    = std::accumulate(scores.begin(), scores.end(), 0.0) / popSize;

  std::vector<int> indices(popSize);
  std::iota(indices.begin(), indices.end(), 0);
  std::sort(indices.begin(), indices.end(),
            [&](int a, int b) { return fitnesses[a] > fitnesses[b]; });

  int eliteCount = std::max(1, static_cast<int>(popSize * eliteFraction));
  std::vector<std::vector<double>> nextGen;
  nextGen.reserve(popSize);

  for (int i = 0; i < eliteCount; ++i)
    nextGen.push_back(genomes[indices[i]]);

  std::uniform_int_distribution<int> parentPick(0, eliteCount - 1);
  std::uniform_real_distribution<double> coin(0.0, 1.0);
  std::normal_distribution<double> noise(0.0, mutationStrength);

  while (static_cast<int>(nextGen.size()) < popSize) {
    std::vector<double> child = genomes[indices[parentPick(rng)]];
    for (auto& w : child) {
      if (coin(rng) < mutationRate) w += noise(rng);
    }
    nextGen.push_back(std::move(child));
  }

  genomes = std::move(nextGen);
  return stats;
}

const std::vector<double>& Population::bestGenome() const {
  int best = 0;
  for (int i = 1; i < popSize; ++i) {
    if (fitnesses[i] > fitnesses[best]) best = i;
  }
  return genomes[best];
}
