#include <cmath>
#include <algorithm>
#include <glm/glm.hpp>

float srgb_to_linear(float c);

float linear_to_srgb(float c);

//Linear vers OK LAB
glm::vec3 linear_srgb_to_oklab(glm::vec3 c);

glm::vec3 oklab_to_linear_srgb(glm::vec3 c);
