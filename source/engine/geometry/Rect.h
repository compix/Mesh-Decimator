#pragma once
#include <glm/glm.hpp>
#include <stdint.h>

class Rect
{
public:
    Rect();
    Rect(const glm::vec2& p1, const glm::vec2& p2);
    Rect(float minX, float minY, float maxX, float maxY);

    void unite(const glm::vec2& p);
    void unite(const Rect& b);

    bool overlaps(const Rect& b) const;
    bool inside(const glm::vec2& p) const;

    void expand(float delta);
    float area() const;

    glm::vec2 center() const { return m_min*0.5f + m_max*0.5f; }

	/**
    * Returns width (x), height (y).
    */
    glm::vec2 scale() const { return m_max - m_min; }

	/**
    * Returns the transformed Rect.
    */
    Rect toWorld(const glm::mat4& world) const;

	const glm::vec2& min() const { return m_min; }
	const glm::vec2& max() const { return m_max; }

    float width() const { return m_max.x - m_min.x; }
    float height() const { return m_max.y - m_min.y; }
    float minX() const { return m_min.x; }
    float minY() const { return m_min.y; }
private:
    glm::vec2 m_min, m_max;
};
