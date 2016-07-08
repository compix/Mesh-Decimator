#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

class Transform
{
public:
    /**
    * Returns the transformation matrix from the local coordinate system of this transform to the world coordinate system.
    */
    glm::mat4 getLocalToWorldMatrix() const;

    /**
    * Returns the transformation matrix from the world coordinate system to the local coordinate system of this transform.
    */
    glm::mat4 getWorldToLocalMatrix() const;

    glm::vec3 getRight() const { return getLocalToWorldRotationMatrix()[0]; }
    glm::vec3 getUp() const { return getLocalToWorldRotationMatrix()[1]; }
    glm::vec3 getForward() const { return getLocalToWorldRotationMatrix()[2]; }

    /**
    * Sets the position in world space.
    */
    void setPosition(const glm::vec3& position);

    /**
    * Returns the position in world space.
    */
    glm::vec3 getPosition() const;

    /**
    * Sets the rotation in world space.
    */
    void setRotation(const glm::quat& rotation);

    /**
    * Returns the rotation quaternion in world space.
    */
    glm::quat getRotation();

    /**
    * Returns the scale in world space. Due to skewing the 3 component scale
    * is only an approximation. An accurate scale is returned if no skew is applied.
    */
    glm::vec3 getApproximateScale();

    void setLocalPosition(const glm::vec3& position);
    void setLocalRotation(const glm::quat& rotation);
    void setLocalScale(const glm::vec3& scale);

    const glm::vec3& getLocalScale() const { return m_scale; }
    const glm::vec3& getLocalPosition() const { return m_position; }
    const glm::quat& getLocalRotation() const { return m_rotation; }

    void lookAt(const glm::vec3& target, const glm::vec3& worldUp = glm::vec3(0.0f, 1.0f, 0.0f));

    /**
    * Set the euler angles in local space.
    * The given angles correspond to a rotation in radians about the matching axis.
    * Rotation order: YXZ
    */
    void setLocalEulerAngles(const glm::vec3& eulerAngles);
    glm::vec3 getLocalEulerAngles() const;

    /**
    * Set the euler angles in world space.
    * The given angles correspond to a rotation in radians about the matching axis.
    * Rotation order: YXZ
    */
    void setEulerAngles(const glm::vec3& eulerAngles);

    /**
    * Returns the rotation as euler angles in world space.
    */
    glm::vec3 getEulerAngles() const;

    void setParent(Transform* parent);

    /**
    * Transforms local point to world space.
    */
    glm::vec3 transformPointToWorld(const glm::vec3& point) const;

    /**
    * Transforms local vector to world space.
    */
    glm::vec3 transformVectorToWorld(const glm::vec3& vector) const;

    /**
    * Transforms world point to local space.
    */
    glm::vec3 transformPointToLocal(const glm::vec3& point) const;

    /**
    * Transforms world vector to local space.
    */
    glm::vec3 transformVectorToLocal(const glm::vec3& vector) const;

    Transform* getRoot() const { return m_root; }

	void pitch(float angle);
	void rotateY(float angle);
private:
    glm::mat4 getLocalMatrix() const;
    glm::mat4 getLocalInverseMatrix() const;

    glm::mat3 getLocalToWorldRotationMatrix() const;
    glm::quat getLocalToWorldRotation() const;
    glm::quat getWorldToLocalRotation() const;
private:
    Transform* m_root{ this };
    Transform* m_parent{ nullptr };
    std::vector<Transform*> m_children;

    // Local coordinate attributes
    glm::vec3 m_position;
    glm::vec3 m_scale{ 1.0f,1.0f,1.0f };
    glm::quat m_rotation;
};
