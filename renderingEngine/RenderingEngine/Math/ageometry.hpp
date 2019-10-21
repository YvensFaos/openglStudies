#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/intersect.hpp> 
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/intersect.hpp>

struct ATriangle {
    glm::vec3 a;
    glm::vec3 b;
    glm::vec3 c;

    ATriangle(void) : a(0.0f, 0.0f, 0.0f), b(0.0f, 0.0f, 0.0f), c(0.0f, 0.0f, 0.0f) {}
    ATriangle(glm::vec3 a, glm::vec3 b, glm::vec3 c) : a(a), b(b), c(c) {}
    ~ATriangle(void) = default;
};

struct ARay {
    glm::vec3 origin;
    glm::vec3 direction;

    ARay(void) : origin(0.0f, 0.0f, 0.0f), direction(0.0f, 0.0f, -1.0f) {}
    ARay(glm::vec3 origin, glm::vec3 direction) : origin(origin), direction(glm::normalize(direction)) {}
    ~ARay(void) = default;

    bool TestARayATriangleIntersection(ATriangle& atriangle, glm::vec2& intersectionPosition, float& distance) const {
        return glm::intersectRayTriangle(origin, direction, atriangle.a, atriangle.b, atriangle.c, intersectionPosition, distance);
    }
};