#pragma once

#include <cstdint>
#include <vector>

class NeuralNet {
  int inputSize;
  int hiddenSize;
  int outputSize;

  std::vector<std::vector<double>> w1;
  std::vector<double>              b1;
  std::vector<std::vector<double>> w2;
  std::vector<double>              b2;

public:
  NeuralNet(int inputSize, int hiddenSize, int outputSize, uint32_t seed);

  std::vector<double> forward(const std::vector<double>& input) const;

  int                 genomeSize() const;
  std::vector<double> getGenome()  const;
  void                setGenome(const std::vector<double>& genome);
};
