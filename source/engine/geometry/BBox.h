#pragma once
#include <glm/glm.hpp>
#include <stdint.h>
#include <initializer_list>

class BBox
{
    friend class SVOBuilder;

public:
    BBox();
    BBox(const glm::vec3& p1, const glm::vec3& p2);

    void unite(const std::initializer_list<glm::vec3>& ps);
    void unite(const glm::vec3& p);
    void unite(const BBox& b);

    bool overlaps(const BBox& b) const;
    bool inside(const glm::vec3& p) const;

    void expand(float delta);
    float surfaceArea() const;
    float volume() const;

    // Returns the number of the longest axis
	// 0 for x, 1 for y and 2 for z
    uint8_t maximumExtent() const;

	glm::vec3 center() const { return m_min*0.5f + m_max*0.5f; }

	// Returns width(x), height(y), depth(z)
	glm::vec3 scale() const { return m_max - m_min; }

	// Returns the transformed BBox.
    BBox toWorld(const glm::mat4& world) const;

    glm::mat4 world() const;

	const glm::vec3& min() const { return m_min; }
	const glm::vec3& max() const { return m_max; }

private:
    glm::vec3 m_min, m_max;
};
