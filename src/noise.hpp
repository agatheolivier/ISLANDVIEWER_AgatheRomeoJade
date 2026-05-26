#pragma once

#include <glm/glm.hpp>
#include <functional>

float perlinNoise(glm::vec2 const& position);
float perlinNoiseSeeded(glm::vec2 const& position, int seed);
float SimplexNoise(glm::vec2 const& position);
float SimplexNoiseSeeded(glm::vec2 const& position, int seed);

float octaveNoise(glm::vec2 const& position, std::function<float(glm::vec2 const&)> noiseFunction, int nboctaves, float lacunarity, float gain, float scale);
