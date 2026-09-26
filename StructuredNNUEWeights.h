#pragma once
#include <cstdint>
#include <string>
#include <vector>

struct StructuredTensor { std::string name; std::vector<std::uint32_t> shape; std::vector<float> data; };
class StructuredNNUEWeights {
public:
    std::vector<StructuredTensor> tensors;
    void Load(const std::string& path);
    std::size_t ParameterCount() const;
};
