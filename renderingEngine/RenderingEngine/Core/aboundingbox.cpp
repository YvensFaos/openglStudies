#include "aboundingbox.hpp"

ABoundingBox::ABoundingBox(void) : min(0.0f, 0.0f, 0.0f), max(0.0f, 0.0f, 0.0f) { }

ABoundingBox::ABoundingBox(glm::vec3 min, glm::vec3 max) : min(min), max(max) { }

ABoundingBox::ABoundingBox(const ABoundingBox& anotherBoundingBox) : min(anotherBoundingBox.getMin()), max(anotherBoundingBox.getMax()) { }

ABoundingBox& ABoundingBox::operator=(const ABoundingBox& anotherBoundingBox) {
    if(this != &anotherBoundingBox) {
        this->min = anotherBoundingBox.getMin();
        this->max = anotherBoundingBox.getMax();
    }
    return *this;
}

glm::vec3 ABoundingBox::getMin() const {
    return this->min;
}

glm::vec3 ABoundingBox::getMax() const {
    return this->max;
}
