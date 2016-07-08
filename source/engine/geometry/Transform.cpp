#include "Transform.h"
#include <engine/util/math.h>

glm::vec3 Transform::getPosition() const
{
    return glm::vec3(getLocalToWorldMatrix()[3]); 
}

glm::mat4 Transform::getLocalToWorldMatrix() const
{
    return m_parent ? m_parent->getLocalToWorldMatrix() * getLocalMatrix() : getLocalMatrix();
}

glm::mat4 Transform::getWorldToLocalMatrix() const
{
    return glm::inverse(getLocalToWorldMatrix());
}

glm::mat3 Transform::getLocalToWorldRotationMatrix() const
{
    return glm::toMat3(getLocalToWorldRotation());
}

glm::quat Transform::getLocalToWorldRotation() const
{
    return m_parent ? m_parent->getLocalToWorldRotation() * m_rotation : m_rotation;
}

glm::quat Transform::getWorldToLocalRotation() const
{
    return glm::inverse(getLocalToWorldRotation());
}

void Transform::setLocalPosition(const glm::vec3& position)
{
    m_position = position;
}

void Transform::setLocalRotation(const glm::quat& rotation)
{
    m_rotation = rotation;
}

void Transform::setPosition(const glm::vec3& position)
{
    m_position = m_parent ? m_parent->transformPointToLocal(position) : position;
}

void Transform::setRotation(const glm::quat& rotation)
{
    m_rotation = m_parent ? m_parent->getWorldToLocalRotation() * rotation : rotation;
}

glm::quat Transform::getRotation()
{
    return getLocalToWorldRotation();
}

glm::vec3 Transform::getApproximateScale()
{
    glm::mat4 world = getLocalToWorldMatrix();

    return glm::vec3(glm::length(world[0]), glm::length(world[1]), glm::length(world[2]));
}

void Transform::setLocalScale(const glm::vec3& scale)
{
    m_scale = scale;
}

void Transform::lookAt(const glm::vec3& target, const glm::vec3& worldUp)
{
    glm::mat3 rotation;
    
    rotation[2] = glm::normalize(target - getPosition());
    rotation[0] = glm::normalize(glm::cross(worldUp, rotation[2]));
    rotation[1] = glm::cross(rotation[2], rotation[0]);
    
    setRotation(glm::toQuat(rotation));
}

void Transform::setLocalEulerAngles(const glm::vec3& eulerAngles)
{
    m_rotation = math::eulerYXZQuat(eulerAngles);
}

glm::vec3 Transform::getLocalEulerAngles() const
{
    return glm::eulerAngles(m_rotation);
}

void Transform::setEulerAngles(const glm::vec3& eulerAngles)
{
    glm::quat rotation = math::eulerYXZQuat(eulerAngles);
    m_rotation = m_parent ? m_parent->getWorldToLocalRotation() * rotation : rotation;
}

glm::vec3 Transform::getEulerAngles() const
{
    return glm::eulerAngles(getLocalToWorldRotation());
}

void Transform::setParent(Transform* parent)
{
    if (m_parent == parent)
        return;

    if (m_parent)
        m_parent->m_children.erase(std::remove(m_children.begin(), m_children.end(), this));

    m_parent = parent;
    m_root = m_parent->m_root;
    m_parent->m_children.push_back(this);
}

glm::vec3 Transform::transformPointToWorld(const glm::vec3& point) const
{
    return glm::vec3(getLocalToWorldMatrix() * glm::vec4(point, 1.0f));
}

glm::vec3 Transform::transformVectorToWorld(const glm::vec3& vector) const
{
    return glm::vec3(getLocalToWorldMatrix() * glm::vec4(vector, 0.0f));
}

glm::vec3 Transform::transformPointToLocal(const glm::vec3& point) const
{
    return glm::vec3(getWorldToLocalMatrix() * glm::vec4(point, 1.0f));
}

glm::vec3 Transform::transformVectorToLocal(const glm::vec3& vector) const
{
    return glm::vec3(getWorldToLocalMatrix() * glm::vec4(vector, 0.0f));
}

glm::mat4 Transform::getLocalMatrix() const
{
    return glm::translate(m_position) * glm::toMat4(m_rotation) * glm::scale(m_scale);
}

glm::mat4 Transform::getLocalInverseMatrix() const
{
    return glm::inverse(getLocalMatrix());
}

void Transform::pitch(float angle)
{
	// Rotate up and look vector about the right vector.
	glm::mat3 rotation;

	rotation[0] = getRight();
	rotation[1] = glm::vec3(glm::rotate(angle, getRight()) * glm::vec4(getUp(), 0));
	rotation[2] = glm::vec3(glm::rotate(angle, getRight()) * glm::vec4(getForward(), 0));

	setRotation(glm::normalize(glm::toQuat(rotation)));
}

void Transform::rotateY(float angle)
{
	// Rotate the basis vectors about the world y-axis.
	glm::mat3 rotation;
	rotation[0] = glm::rotateY(getRight(), angle);
	rotation[1] = glm::rotateY(getUp(), angle);
	rotation[2] = glm::rotateY(getForward(), angle);

	setRotation(glm::normalize(glm::toQuat(rotation)));
}
