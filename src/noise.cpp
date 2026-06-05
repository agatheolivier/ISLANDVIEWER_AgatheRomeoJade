
#include "noise.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>

#include <cstdint>
#include <functional>

namespace {

// Fast integer hash function
std::uint32_t hashU32(std::uint32_t v)
{
    v ^= v >> 16;
    v *= 0x7feb352du;
    v ^= v >> 15;
    v *= 0x846ca68bu;
    v ^= v >> 16;
    return v;
}

glm::vec2 seedToOffset2D(int seed)
{
    std::uint32_t const base { static_cast<std::uint32_t>(seed) };
    std::uint32_t const hx { hashU32(base ^ 0x9e3779b9u) };
    std::uint32_t const hy { hashU32(base ^ 0x85ebca6bu) };

    float const fx { static_cast<float>(hx & 0x00ffffffu) / 16777216.0f };
    float const fy { static_cast<float>(hy & 0x00ffffffu) / 16777216.0f };

    // Large translation range so seeds land on very different 2D Perlin regions.
    return {
        fx * 4096.0f - 2048.0f,
        fy * 4096.0f - 2048.0f
    };
}

} // namespace

float Simplex(const glm::vec2 &p){

    //skewed
    glm::vec2 skew = glm::vec2(p.x+p.y*0.5, p.y + 0.001); 
    glm::vec2 losange = floor(skew); 
    glm::vec2 triangle = fract(skew); 

    //Determination si on est dans un triangle inferieur ou exterieur 
    glm::vec2 offset;
    if (triangle.y < triangle.x)
        offset = glm::vec2(1.0f, 0.0f);
    else
        offset = glm::vec2(0.0f, 1.0f); 

    // 3 coins du triangle
    glm::vec2 coin0 = losange;
    glm::vec2 coin1 = losange + offset;
    glm::vec2 coin2 = losange + glm::vec2(1.0f, 1.0f);

    //On repasse dans l'espace normal
    glm::vec2 pos0 = glm::vec2(coin0.x - coin0.y * 0.5f, coin0.y);
    glm::vec2 pos1 = glm::vec2(coin1.x - coin1.y * 0.5f, coin1.y);
    glm::vec2 pos2 = glm::vec2(coin2.x - coin2.y * 0.5f, coin2.y);


    // hashage tiré d'internet 
    glm::vec2 i1 = losange + offset;
    glm::vec2 i2 = losange + 1.0f; 
    glm::vec3 iu = glm::vec3(losange.x, i1.x, i2.x); 
    glm::vec3 iv = glm::vec3(losange.y, i1.y, i2.y);
    glm::vec3 hash = glm::mod(iu, 289.0f); 
    hash = glm::mod((hash*51.0f + 2.0f)*hash + iv, 289.0f); 
    hash = glm::mod((hash*34.0f + 10.0f)*hash, 289.0f);     
    glm::vec3 psi = hash*0.07482f;
    glm::vec3 gx = cos(psi); 
    glm::vec3 gy = sin(psi);
    glm::vec2 g0 = glm::vec2(gx.x, gy.x);
    glm::vec2 g1 = glm::vec2(gx.y, gy.y);
    glm::vec2 g2 = glm::vec2(gx.z, gy.z);


    // Poids attribués en fonction des distances de chaque coin par rapport à p
    glm::vec2 d0 = p - pos0;
    glm::vec2 d1 = p - pos1;
    glm::vec2 d2 = p - pos2;

    float w0 = glm::max(1.0f - glm::length(d0), 0.0f);
    float w1 = glm::max(1.0f - glm::length(d1), 0.0f);
    float w2 = glm::max(1.0f - glm::length(d2), 0.0f);

    //Contribution de chaque coin 
    float n = w0 * glm::dot(g0, d0)
            + w1 * glm::dot(g1, d1)
            + w2 * glm::dot(g2, d2);

    return  n;
}

float SimplexNoise(glm::vec2 const& position) {
    return Simplex(position);
}

float perlinNoise(glm::vec2 const& position) {
    return glm::perlin(position);
}

float SimplexNoiseSeeded (glm::vec2 const& position, int seed) {
    // Cache computed offset because the same seed is used for many samples per frame.
    static int cachedSeed {};
    static glm::vec2 cachedOffset {};

    if (seed != cachedSeed) {
        cachedSeed = seed;
        cachedOffset = seedToOffset2D(seed);
    }

    return Simplex(position + cachedOffset);
}

float perlinNoiseSeeded (glm::vec2 const& position, int seed) {
    // Cache computed offset because the same seed is used for many samples per frame.
    static int cachedSeed {};
    static glm::vec2 cachedOffset {};

    if (seed != cachedSeed) {
        cachedSeed = seed;
        cachedOffset = seedToOffset2D(seed);
    }

    return glm::perlin(position + cachedOffset);
}

float octaveNoise (glm::vec2 const& position, std::function<float(glm::vec2 const&)> noiseFunction, int nboctaves, float lacunarity, float gain, float scale) {
    float value = {};
    float amplitude = 1.0f;
    float frequency = 1.0;
    float totalAmplitude = 0.0f;
    glm::vec2 scaledPos = position * scale;
    for (int i = 0; i < nboctaves; i++) {

        value += amplitude * noiseFunction((scaledPos*scale) * frequency);
        totalAmplitude += amplitude;
        frequency *= lacunarity;
        amplitude *= gain;
    }

    return value/totalAmplitude;
}

