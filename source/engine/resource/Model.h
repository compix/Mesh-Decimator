#pragma once
#include <vector>
#include <engine/geometry/Transform.h>
#include <engine/rendering/geometry/Mesh.h>
#include <memory>

struct Model
{
    void addChild(std::shared_ptr<Model> model);
    std::vector<Mesh::SubMesh> getAllSubMeshes() const;
    const Mesh::SubMesh& getSubMesh(size_t idx) { return subMeshes[idx]; }

    void mapToUnitCube();

	// Members
	glm::vec3 position;
	glm::vec3 scale{ 1.0f,1.0f,1.0f };
	glm::quat rotation;

    // Index of sub mesh corresponds to the index of the material
	std::vector<Mesh::SubMesh> subMeshes;

    std::vector<std::shared_ptr<Model>> children;
    Model* parent{ nullptr };
};
