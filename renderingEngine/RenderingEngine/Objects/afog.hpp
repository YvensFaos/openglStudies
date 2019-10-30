#pragma once

#include <glm/glm.hpp>

struct AFog {
    public:
        float maxDist;
        float minDist;
        glm::vec4 color;

    public:
        AFog(float maxDist, float minDist, glm::vec4 color) : maxDist(maxDist), minDist(minDist), color(color) { }
};