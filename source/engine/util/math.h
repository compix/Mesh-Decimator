#pragma once
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

struct Polar
{
	float radius;
	float elevation;
	float azimuth;

	Polar(float radius, float elevation, float azimuth)
		:radius(radius), elevation(elevation), azimuth(azimuth)
	{
	}
};

namespace math
{
	const float PI = 3.14159265359f;
	const float EPSILON = 0.0000001f;
    const float EPSILON5 = 10e-5f;
	const float TO_DEG = 180.f / PI;
	const float TO_RAD = PI / 180.f;

    const float PI2 = PI * 2.0f;
    const float PI_DIV_2  = PI * 0.5f;
    const float PI_DIV_4  = PI * 0.25f;

    void rotate(glm::vec2& p, const float&  angle);
	inline float lerp(float start, float end, float t) { return (1 - t)*start + t*end; }
    glm::vec2 lerp(const glm::vec2& start, const glm::vec2& end, float t);

    template<class T>
	T clamp(T v, T min, T max) { return v < min ? min : v > max ? max : v; }

	inline float smoothstep(float start, float end, float t) { return start + t*t*(3 - 2 * t) * (end - start); }
    glm::vec2 smoothstep(const glm::vec2& start, const glm::vec2& end, float t);

	inline float smootherstep(float start, float end, float t) { return start + t*t*t*(t*(t * 6 - 15) + 10) * (end-start); }

	inline float regress(float start, float end, float t) { return start + (sqrtf(t)*2.f - t)*(end-start); }
    glm::vec2 regress(const glm::vec2& start, const glm::vec2& end, float t);

	inline float min(float v0, float v1, float v2) { return std::min(v0, std::min(v1, v2)); }
	inline float max(float v0, float v1, float v2) { return std::max(v0, std::max(v1, v2)); }

	inline float toRadians(float degree) { return degree * PI / 180.f; }
	inline float toDegrees(float radians) { return radians * 180.f / PI; }

    inline bool nearEq(float n1, float n2) { return std::abs(n1 - n2) <= EPSILON; }
    inline bool nearEq(float n1, float n2, float epsilon) { return std::abs(n1 - n2) <= epsilon; }

    inline glm::mat4 scale(float scale) { return glm::scale(glm::vec3(scale, scale, scale)); }
    glm::mat4 rotationX(float angle);
    glm::mat4 rotationY(float angle);
    glm::mat4 rotationZ(float angle);

    inline glm::mat4 orthoLH(float left, float right, float bottom, float top, float zNear, float zFar)
    {
        glm::mat4 result(1.f);
        result[0][0] = 2.f / (right - left);
        result[1][1] = 2.f / (top - bottom);
        result[2][2] = 2.f / (zFar - zNear);
        result[3][0] = (right + left) / (left - right);
        result[3][1] = (top + bottom) / (bottom - top);
        result[3][2] = (zFar + zNear) / (zNear - zFar);
        return result;
    }

    inline glm::mat4 identityMatrix()
    {
        return glm::mat4(
            1.f, 0.f, 0.f, 0.f,
            0.f, 1.f, 0.f, 0.f,
            0.f, 0.f, 1.f, 0.f,
            0.f, 0.f, 0.f, 1.f);
    }

    inline glm::mat4 rotationX90Deg()
    {
        return glm::mat4(1.f, 0.f, 0.f, 0.f,
                         0.f, 0, 1.f, 0.f,
                         0.f, -1.f, 0, 0.f,
                         0.f, 0.f, 0.f, 1.f);
    }

    inline glm::mat4 rotationY90Deg()
    {
        return glm::mat4(0.f, 0.f, -1.f, 0.f,
                         0.f, 1.f, 0.f, 0.f,
                         1.f, 0.f, 0.f, 0.f,
                         0.f, 0.f, 0.f, 1.f);
    }

    inline glm::mat4 rotationZ90Deg()
    {
        return glm::mat4(0.f, 1.f, 0.f, 0.f,
                         -1.f, 0.f, 0.f, 0.f,
                         0.f, 0.f, 1.f, 0.f,
                         0.f, 0.f, 0.f, 1.f);
    }
    inline glm::mat4 rotationXMinus90Deg()
    {
        return glm::mat4(1.f, 0.f, 0.f, 0.f,
                         0.f, 0, -1.f, 0.f,
                         0.f, 1.f, 0, 0.f,
                         0.f, 0.f, 0.f, 1.f);
    }

    inline glm::mat4 rotationYMinus90Deg()
    {
        return glm::mat4(0.f, 0.f, 1.f, 0.f,
                         0.f, 1.f, 0.f, 0.f,
                         -1.f, 0.f, 0.f, 0.f,
                         0.f, 0.f, 0.f, 1.f);
    }

    inline glm::mat4 rotationZMinus90Deg()
    {
        return glm::mat4(0.f, -1.f, 0.f, 0.f,
                         1.f, 0.f, 0.f, 0.f,
                         0.f, 0.f, 1.f, 0.f,
                         0.f, 0.f, 0.f, 1.f);
    }

    /**
    * Rotation by y radians about y axis followed by 
    * rotation by x radians about x axis followed by 
    * rotation by z radians about z axis.
    */
    glm::quat eulerYXZQuat(const glm::vec3& eulerAngles);

    /**
    * Rotation by yaw radians about y axis followed by
    * rotation by pitch radians about x axis followed by
    * rotation by roll radians about z axis.
    */
    glm::quat eulerYXZQuat(float yaw, float pitch, float roll);

    /**
    * Rotation by yaw radians about y axis followed by
    * rotation by pitch radians about x axis followed by
    * rotation by roll radians about z axis.
    */
    glm::quat yawPitchRoll(float yaw, float pitch, float roll);

	inline glm::vec3 polarToVector(float radius, float elevation, float azimuth)
	{
		return glm::vec3(-radius * cosf(elevation) * sinf(azimuth),
			radius * sinf(elevation),
			radius * cosf(elevation) * cosf(azimuth));
	}

	inline Polar vectorToPolar(glm::vec3 v)
	{
		float r = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
		return Polar(r, glm::asin(v.y / r), -glm::atan(v.x, v.z));
	}
}
