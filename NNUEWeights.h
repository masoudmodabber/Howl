#pragma once
#include <cstdint>
#include <string>
#include <vector>

struct NNUETensor { std::string name; std::vector<std::uint32_t> shape; std::vector<float> data; };
class NNUEWeights {
public:
    std::vector<NNUETensor> tensors;
    void Load(const std::string& path);
    std::size_t ParameterCount() const;
};
