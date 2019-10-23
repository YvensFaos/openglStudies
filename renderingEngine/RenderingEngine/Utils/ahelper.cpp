#include "ahelper.hpp"

#include <algorithm>

glm::vec4 AHelper::generateRandomColor(bool generateAlpha) {
    glm::vec4 randomColor;
    randomColor.x = (rand() % 256) / 255.0f;
    randomColor.y = (rand() % 256) / 255.0f;
    randomColor.z = (rand() % 256) / 255.0f;
    randomColor.w = (generateAlpha) ? rand() % 256 : 1.0;
    return randomColor;
}