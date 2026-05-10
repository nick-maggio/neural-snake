#include "neural_net.h"
#include <random>
#include <stdexcept>

static double relu(double x) { return x > 0.0 ? x : 0.0; }

NeuralNet::NeuralNet(int inputSize, int hiddenSize, int outputSize, uint32_t seed)
  : inputSize(inputSize), hiddenSize(hiddenSize), outputSize(outputSize) {
  std::mt19937 rng(seed);
  std::uniform_real_distribution<double> dist(-1.0, 1.0);

  w1.assign(hiddenSize, std::vector<double>(inputSize));
  b1.resize(hiddenSize);
  w2.assign(outputSize, std::vector<double>(hiddenSize));
  b2.resize(outputSize);

  for (auto& row : w1) for (auto& v : row) v = dist(rng);
  for (auto& v : b1)                        v = dist(rng);
  for (auto& row : w2) for (auto& v : row) v = dist(rng);
  for (auto& v : b2)                        v = dist(rng);
}

int NeuralNet::genomeSize() const {
  return inputSize * hiddenSize + hiddenSize
       + hiddenSize * outputSize + outputSize;
}

std::vector<double> NeuralNet::getGenome() const {
  std::vector<double> g;
  g.reserve(genomeSize());
  for (const auto& row : w1) for (double v : row) g.push_back(v);
  for (double v : b1)                              g.push_back(v);
  for (const auto& row : w2) for (double v : row) g.push_back(v);
  for (double v : b2)                              g.push_back(v);
  return g;
}

void NeuralNet::setGenome(const std::vector<double>& g) {
  if (static_cast<int>(g.size()) != genomeSize())
    throw std::runtime_error("Genome size mismatch");

  int idx = 0;
  for (auto& row : w1) for (auto& v : row) v = g[idx++];
  for (auto& v : b1)                        v = g[idx++];
  for (auto& row : w2) for (auto& v : row) v = g[idx++];
  for (auto& v : b2)                        v = g[idx++];
}

std::vector<double> NeuralNet::forward(const std::vector<double>& input) const {
  if (static_cast<int>(input.size()) != inputSize)
    throw std::runtime_error("Input size mismatch in NeuralNet::forward");

  std::vector<double> hidden(hiddenSize, 0.0);
  for (int i = 0; i < hiddenSize; ++i) {
    double sum = b1[i];
    for (int j = 0; j < inputSize; ++j) sum += w1[i][j] * input[j];
    hidden[i] = relu(sum);
  }

  std::vector<double> output(outputSize, 0.0);
  for (int i = 0; i < outputSize; ++i) {
    double sum = b2[i];
    for (int j = 0; j < hiddenSize; ++j) sum += w2[i][j] * hidden[j];
    output[i] = sum;
  }

  return output;
}
