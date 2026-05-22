#include "population.h"
#include "game_state.h"
#include <algorithm>
#include <numeric>
#include <stdexcept>
#include <thread>
#include <vector>

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
  gameSeeds.assign(popSize, 0u);
}

GenerationStats Population::runGeneration() {
  std::vector<int> scores(popSize, 0);

  // Pre-generate seeds on the main thread so workers never touch the master rng —
  // keeps evaluation order deterministic for a given master seed.
  std::vector<uint32_t> agentSeeds(popSize);
  for (int i = 0; i < popSize; ++i) {
    agentSeeds[i] = static_cast<uint32_t>(rng());
    gameSeeds[i]  = static_cast<uint32_t>(rng());
  }

  unsigned numThreads = std::thread::hardware_concurrency();
  if (numThreads == 0) numThreads = 4; // fallback if detection fails
  if ((int)numThreads > popSize) numThreads = popSize;

  auto worker = [&](int start, int end) {
    for (int i = start; i < end; ++i) {
      NeuralAgent agent(agentSeeds[i]);
      agent.getNet().setGenome(genomes[i]);

      GameState        game(boardWidth, boardHeight, gameSeeds[i]);
      SimulationResult r = simulate(game, agent);

      fitnesses[i] = r.fitness;
      scores[i]    = r.score;
    }
  };

  std::vector<std::thread> threads;
  threads.reserve(numThreads);

  int chunk = popSize / (int)numThreads;
  int remainder = popSize % (int)numThreads;
  int cursor = 0;
  for (unsigned t = 0; t < numThreads; ++t) {
    // Distribute remainder across the first 'remainder' threads so chunk
    // sizes differ by at most 1.
    int size = chunk + ((int)t < remainder ? 1 : 0);
    int start = cursor;
    int end   = cursor + size;
    cursor    = end;
    threads.emplace_back(worker, start, end);
  }
  for (auto& th : threads) th.join();

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

std::vector<int> Population::topNIndices(int n) const {
  if (n > popSize) n = popSize;
  if (n < 0)       n = 0;

  std::vector<int> indices(popSize);
  std::iota(indices.begin(), indices.end(), 0);

  std::partial_sort(indices.begin(), indices.begin() + n, indices.end(),
                    [&](int a, int b) { return fitnesses[a] > fitnesses[b]; });

  indices.resize(n);
  return indices;
}