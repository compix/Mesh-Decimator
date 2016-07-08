#include "DirectedEdgeMesh.h"
#include <unordered_map>

DirectedEdgeMesh::DirectedEdgeMesh(const Mesh::SubMesh& subMesh)
{
    assert(subMesh.vertices.size() > 0);
    assert(subMesh.indices.size() > 0);

    m_subMesh = subMesh;
    m_vertices.resize(subMesh.vertices.size());
    m_edges.resize(subMesh.indices.size());

    std::unordered_map<uint64_t, EdgeID> halfedgeMap;
    size_t halfedgeCount = m_subMesh.indices.size();

    for (size_t i = 0; i < halfedgeCount; ++i)
    {
        VertexID vID = m_subMesh.indices[i];
        auto& halfedge = m_edges[i];
        halfedge.vertexIdx = vID;
        m_vertices[vID].id = vID;
        m_vertices[vID].edgeID = EdgeID(i);

        auto nextIdx = next(i);
        VertexID nextVID = m_subMesh.indices[nextIdx];

        auto edgeKey = uint64_t(vID) * halfedgeCount + uint64_t(nextVID);

        // 2 Halfedges from v0 to v1 should not exist (non-manifold)
        assert(halfedgeMap.count(edgeKey) == 0);

        halfedgeMap[edgeKey] = EdgeID(i);

        auto oppositeKey = uint64_t(nextVID) * halfedgeCount + uint64_t(vID);
        auto it = halfedgeMap.find(oppositeKey);
        if (it != halfedgeMap.end())
        {
            halfedge.opposite = it->second;
            m_edges[halfedge.opposite].opposite = i;
        }
    }

    for (size_t i = 0; i < m_edges.size(); ++i)
    {
        if (m_edges[i].opposite >= 0)
            assert(m_edges[m_edges[i].opposite].opposite == EdgeID(i));
    }

    // Set up border vertices
    for (size_t i = 0; i < m_edges.size(); ++i)
    {
        if (m_edges[i].opposite < 0)
        {
            if (m_vertices[m_edges[i].vertexIdx].id < 0)
                continue;

            auto e = findEmanatingEdges(m_edges[i].vertexIdx);
            m_emanatingEdges.push_back(e);
            m_vertices[m_edges[i].vertexIdx].id = -VertexID(m_emanatingEdges.size());
        }
    }
}

uint32_t DirectedEdgeMesh::valenceOf(VertexIndex vertexIdx)
{
    return getNeighbors(vertexIdx).size();
}

std::vector<VertexIndex> DirectedEdgeMesh::getNeighbors(VertexIndex vertexIdx)
{
    auto vID = m_vertices[vertexIdx].id;

    std::vector<VertexIndex> adjacent;
    if (vID < 0)
    {
        auto& emanatingEdges = m_emanatingEdges[-vID - 1];
        for (auto i : emanatingEdges)
        {
            addIfNew(adjacent, m_edges[next(i)].vertexIdx);
            addIfNew(adjacent, m_edges[prev(i)].vertexIdx);
        }

        return adjacent;
    }

    EdgeID startIdx = m_vertices[vertexIdx].edgeID;
    EdgeID curIndex = startIdx;

    do
    {
        assert(m_edges[curIndex].opposite >= 0);
        curIndex = next(m_edges[curIndex].opposite);
        adjacent.push_back(m_edges[m_edges[curIndex].opposite].vertexIdx);
    } while (curIndex != startIdx);

    return adjacent;
}

std::vector<EdgeID> DirectedEdgeMesh::findEmanatingEdges(VertexIndex vIdx)
{
    std::vector<EdgeID> emanating;

    size_t i = 0;
    while (i < m_edges.size())
    {
        if (vIdx == VertexIndex(m_edges[i].vertexIdx))
        {
            emanating.push_back(i);
            i = ((i + 3) / 3) * 3;
        }
        else
            ++i;
    }

    return emanating;
}

std::vector<EdgeID> DirectedEdgeMesh::getEmanatingEdges(VertexIndex vIdx)
{
    if (m_vertices[vIdx].id < 0)
        return m_emanatingEdges[-m_vertices[vIdx].id - 1];

    std::vector<EdgeID> emanating;
    EdgeID startIdx = m_vertices[vIdx].edgeID;
    EdgeID curIndex = startIdx;

    do
    {
        assert(m_edges[curIndex].opposite >= 0);
        curIndex = next(m_edges[curIndex].opposite);
        emanating.push_back(curIndex);
    } while (curIndex != startIdx);

    return emanating;
}

std::vector<FaceIndex> DirectedEdgeMesh::getAdjacentFaces(VertexIndex vIdx)
{
    auto emanatingEdges = getEmanatingEdges(vIdx);

    std::vector<FaceIndex> faces;

    for (auto i : emanatingEdges)
        faces.push_back(i / 3);

    return faces;
}

glm::vec3 DirectedEdgeMesh::computeFaceNormal(FaceIndex faceIdx)
{
    auto edgeStart = faceIdx * 3;
    auto v0 = m_subMesh.vertices[m_edges[edgeStart].vertexIdx];
    auto v1 = m_subMesh.vertices[m_edges[edgeStart + 1].vertexIdx];
    auto v2 = m_subMesh.vertices[m_edges[edgeStart + 2].vertexIdx];

    return glm::normalize(glm::cross(v1 - v0, v2 - v0));
}

glm::vec3 DirectedEdgeMesh::computeVertexNormal(VertexIndex vIdx)
{
    auto adjFaces = getAdjacentFaces(vIdx);
    glm::vec3 normal(0.f);

    for (auto faceIdx : adjFaces)
        normal += computeFaceNormal(faceIdx);

    return glm::normalize(normal);
}

std::vector<VertexIndex> DirectedEdgeMesh::findNeighbors(VertexIndex vID)
{
    std::vector<VertexIndex> neighbors;
    auto& indices = m_subMesh.indices;

    size_t i = 0;
    while (i < m_subMesh.indices.size())
    {
        if (vID == VertexIndex(m_subMesh.indices[i]))
        {
            addIfNew(neighbors, indices[next(i)]);
            addIfNew(neighbors, indices[prev(i)]);
            i = ((i + 3) / 3) * 3;
        }
        else
            ++i;
    }

    return neighbors;
}

bool DirectedEdgeMesh::doesVertexBelongToFace(FaceIndex faceIdx, VertexIndex vertexIdx)
{
    EdgeID i = faceIdx * 3;
    return (m_edges[i].vertexIdx == vertexIdx ||
        m_edges[i + 1].vertexIdx == vertexIdx ||
        m_edges[i + 2].vertexIdx == vertexIdx);
}
