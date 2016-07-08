#include "math.h"

void math::rotate(glm::vec2& p, const float& angle)
{
	const float x = p.x;
	const float c = cosf(angle);
	const float s = sinf(angle);

	p.x = x*c - p.y*s;
	p.y = p.y*c + x*s;
}

glm::vec2 math::lerp(const glm::vec2& start, const glm::vec2& end, float t)
{
    glm::vec2 result;
    result.x = math::lerp(start.x, end.x, t);
    result.y = math::lerp(start.y, end.y, t);

	return result;
}

glm::vec2 math::smoothstep(const glm::vec2& start, const glm::vec2& end, float t)
{
    glm::vec2 result;
	float m = t*t*(3 - 2 * t);
	result.x = start.x + m*(end.x - start.x);
	result.y = start.y + m*(end.y - start.y);

	return result;
}

glm::vec2 math::regress(const glm::vec2& start, const glm::vec2& end, float t)
{
    glm::vec2 result;
	float m = sqrtf(t)*2.f - t;
	result.x = start.x + m*(end.x - start.x);
	result.y = start.y + m*(end.y - start.y);

	return result;
}

glm::mat4 math::rotationX(float angle)
{
    float c = cosf(angle);
    float s = sinf(angle);
    return glm::mat4(1.f, 0.f, 0.f, 0.f,
                     0.f, c, s, 0.f,
                     0.f, -s, c, 0.f,
                     0.f, 0.f, 0.f, 1.f);
}

glm::mat4 math::rotationY(float angle)
{
    float c = cosf(angle);
    float s = sinf(angle);
    return glm::mat4(c, 0.f, -s, 0.f,
                     0.f, 1.f, 0.f, 0.f,
                     s, 0.f, c, 0.f,
                     0.f, 0.f, 0.f, 1.f);
}

glm::mat4 math::rotationZ(float angle)
{
    float c = cosf(angle);
    float s = sinf(angle);
    return glm::mat4(c, s, 0.f, 0.f,
                     -s, c, 0.f, 0.f,
                     0.f, 0.f, 1.f, 0.f,
                     0.f, 0.f, 0.f, 1.f);
}

glm::quat math::eulerYXZQuat(const glm::vec3& eulerAngles)
{
    return eulerYXZQuat(eulerAngles.y, eulerAngles.x, eulerAngles.z);
}

glm::quat math::eulerYXZQuat(float yaw, float pitch, float roll)
{
    float cy = std::cosf(yaw * 0.5f);
    float sy = std::sinf(yaw * 0.5f);

    float cp = std::cosf(pitch * 0.5f);
    float sp = std::sinf(pitch * 0.5f);

    float cr = std::cosf(roll * 0.5f);
    float sr = std::sinf(roll * 0.5f);

    return glm::quat(cy*cp*cr + sy*sp*sr,
                     cy*sp*cr + sy*cp*sr,
                     sy*cp*cr - cy*sp*sr,
                     cy*cp*sr - sy*sp*cr);
}

glm::quat math::yawPitchRoll(float yaw, float pitch, float roll)
{
    return eulerYXZQuat(yaw, pitch, roll);
}
