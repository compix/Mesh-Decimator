#include "Model.h"
#include <engine/util/util.h>

void Model::addChild(std::shared_ptr<Model> model)
{
    children.push_back(model);
    model->parent = this;
}

std::vector<Mesh::SubMesh> Model::getAllSubMeshes() const
{
    std::vector<Mesh::SubMesh> allMeshes;
    allMeshes.insert(allMeshes.end(), subMeshes.begin(), subMeshes.end());

    for (auto m : children)
    {
        auto childMeshes = m->getAllSubMeshes();
        allMeshes.insert(allMeshes.end(), childMeshes.begin(), childMeshes.end());
    }

    return allMeshes;
}

void computeBBox(Model* model, BBox& bbox)
{
    for (auto& subMesh : model->subMeshes)
        for (auto& v : subMesh.vertices)
            bbox.unite(v);

    for (auto& child : model->children)
        computeBBox(child.get(), bbox);
}

void mapToUnitCubeImpl(Model* model, const BBox& bbox)
{
    float scaleInv = 1.0f / bbox.scale()[bbox.maximumExtent()];
    glm::vec3 offset = -bbox.min();

    for (auto& subMesh : model->subMeshes)
        for (size_t i = 0; i < subMesh.vertices.size(); ++i)
            subMesh.vertices[i] = (subMesh.vertices[i] + offset) * scaleInv;

    for (auto& child : model->children)
        mapToUnitCubeImpl(child.get(), bbox);
}

void Model::mapToUnitCube()
{
    BBox bbox;
    computeBBox(this, bbox);
    mapToUnitCubeImpl(this, bbox);
}
