#pragma once
#include <vector>
#include <engine/rendering/geometry/Mesh.h>
#include <set>

using VertexID = int32_t;
using EdgeID = int32_t;
using VertexIndex = uint32_t;
using FaceIndex = uint32_t;

#define INVALID_EDGE_ID std::numeric_limits<EdgeID>().max()
#define INVALID_VERTEX_INDEX std::numeric_limits<VertexIndex>().max()
#define INVALID_VERTEX_ID std::numeric_limits<VertexID>().max()
#define BOUNDARY_EDGE -1

struct Halfedge
{
    VertexIndex vertexIdx{ INVALID_VERTEX_INDEX };
    EdgeID opposite{ BOUNDARY_EDGE };
};

struct HalfedgeVertex
{
    VertexID id{ INVALID_VERTEX_ID };
    EdgeID edgeID{ INVALID_EDGE_ID };
};

/**
* Note: getNeighbors, getEmanatingEdges, getAdjacentFaces and set operations add a lot of allocation and deallocation overhead.
* Preallocated pools would increase the speed of the algorithm considerably.
*/
class DirectedEdgeMesh
{
public:
    /**
    * Expecting a 2-manifold mesh (optionally with borders) as input. 
    * Note: Behaviour for non-manifold geometry is undefined.
    */
    explicit DirectedEdgeMesh(const Mesh::SubMesh& subMesh);
    DirectedEdgeMesh() {}
    virtual ~DirectedEdgeMesh() {}

    const std::vector<HalfedgeVertex>& getVertices() const { return m_vertices; }
    const std::vector<Halfedge>& getEdges() const { return m_edges; }

    template<class T>
    static T next(T idx);

    template<class T>
    static T prev(T idx);

    uint32_t valenceOf(VertexIndex vertexIdx);

    std::vector<VertexIndex> getNeighbors(VertexIndex vertexIdx);

    std::vector<VertexIndex> findNeighbors(VertexIndex vID);

    template<class T>
    void addIfNew(std::vector<T>& v, const T& elem);

    bool doesVertexBelongToFace(FaceIndex faceIdx, VertexIndex vertexIdx);

    std::vector<EdgeID> getEmanatingEdges(VertexIndex vIdx);
    std::vector<FaceIndex> getAdjacentFaces(VertexIndex vIdx);

    glm::vec3 computeFaceNormal(FaceIndex faceIdx);
    glm::vec3 computeVertexNormal(VertexIndex vIdx);

    const Mesh::SubMesh& getSubMesh() const { return m_subMesh; }

protected:
    std::vector<EdgeID> findEmanatingEdges(VertexIndex vIdx);

protected:
    Mesh::SubMesh m_subMesh;

    std::vector<HalfedgeVertex> m_vertices;
    std::vector<Halfedge> m_edges;
    std::vector<std::vector<EdgeID>> m_emanatingEdges;
};

template <class T>
T DirectedEdgeMesh::next(T idx)
{
    return (idx / 3) * 3 + (idx + 1) % 3;
}

template <class T>
T DirectedEdgeMesh::prev(T idx)
{
    return (idx / 3) * 3 + (idx + 2) % 3;
}

template <class T>
void DirectedEdgeMesh::addIfNew(std::vector<T>& v, const T& elem)
{
    if (std::find(v.begin(), v.end(), elem) == v.end())
        v.push_back(elem);
}
