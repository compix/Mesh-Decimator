#include "BBox.h"
#include <algorithm>
#include <glm/ext.hpp>

BBox::BBox()
    :m_min(FLT_MAX, FLT_MAX, FLT_MAX),
     m_max(-FLT_MAX, -FLT_MAX, -FLT_MAX)
{
}

BBox::BBox(const glm::vec3& p1, const glm::vec3& p2)
    :m_min(std::min(p1.x, p2.x), std::min(p1.y, p2.y), std::min(p1.z, p2.z)),
     m_max(std::max(p1.x, p2.x), std::max(p1.y, p2.y), std::max(p1.z, p2.z))
{
}

void BBox::unite(const std::initializer_list<glm::vec3>& ps)
{
    for (auto& p : ps)
        unite(p);
}

void BBox::unite(const glm::vec3& p)
{
    m_min.x = std::min(m_min.x, p.x);
    m_min.y = std::min(m_min.y, p.y);
    m_min.z = std::min(m_min.z, p.z);

    m_max.x = std::max(m_max.x, p.x);
    m_max.y = std::max(m_max.y, p.y);
    m_max.z = std::max(m_max.z, p.z);
}

void BBox::unite(const BBox& b)
{
    m_min.x = std::min(m_min.x, b.m_min.x);
    m_min.y = std::min(m_min.y, b.m_min.y);
    m_min.z = std::min(m_min.z, b.m_min.z);

    m_max.x = std::max(m_max.x, b.m_max.x);
    m_max.y = std::max(m_max.y, b.m_max.y);
    m_max.z = std::max(m_max.z, b.m_max.z);
}

bool BBox::overlaps(const BBox& b) const
{
    return ((m_max.x >= b.m_min.x) && (m_min.x <= b.m_max.x) &&
            (m_max.y >= b.m_min.y) && (m_min.y <= b.m_max.y) &&
            (m_max.z >= b.m_min.z) && (m_min.z <= b.m_max.z));
}

bool BBox::inside(const glm::vec3& p) const
{
    return (p.x >= m_min.x && p.x <= m_max.x &&
            p.y >= m_min.y && p.y <= m_max.y &&
            p.z >= m_min.z && p.z <= m_max.z);
}

void BBox::expand(float delta)
{
    m_min -= glm::vec3(delta, delta, delta);
    m_max += glm::vec3(delta, delta, delta);
}

float BBox::surfaceArea() const
{
    auto d = m_max - m_min;
	return 2.f * (d.x * d.y + d.x * d.z + d.y * d.z);
}

float BBox::volume() const
{
    auto d = m_max - m_min;
	return d.x * d.y * d.z;
}

uint8_t BBox::maximumExtent() const
{
    auto d = m_max - m_min;
    if (d.x > d.y && d.x > d.z)
		return 0;

    if (d.y > d.z)
		return 1;

	return 2;
}

BBox BBox::toWorld(const glm::mat4& world) const
{
    return BBox(glm::vec3(world * glm::vec4(m_min, 1.f)), glm::vec3(world * glm::vec4(m_max, 1.f)));
}

glm::mat4 BBox::world() const
{
    return glm::translate(center()) * glm::scale(scale());
}

