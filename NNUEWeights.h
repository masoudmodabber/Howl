#pragma once
#include <cstdint>
#include <string>
#include <vector>

#include <iosfwd>

struct NNUETensor { std::string name; std::vector<std::uint32_t> shape; std::vector<float> data; };
class NNUEWeights {
public:
    std::vector<NNUETensor> tensors;
    void Load(const std::string& path);
    void Load(std::istream& f);
    void LoadFromMemory(const void* data, std::size_t size);
    std::size_t ParameterCount() const;
};
