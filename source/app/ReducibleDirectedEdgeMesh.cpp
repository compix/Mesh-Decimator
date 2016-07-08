#include "ReducibleDirectedEdgeMesh.h"
#include <algorithm>
#include <engine/util/set_operations.h>

ReducibleDirectedEdgeMesh::ReducibleDirectedEdgeMesh(const Mesh::SubMesh& subMesh)
    :DirectedEdgeMesh(subMesh)
{
    m_removedFaces.resize(m_edges.size() / 3);
    std::fill(m_removedFaces.begin(), m_removedFaces.end(), false);

    initCollapseCandidates();
}

void ReducibleDirectedEdgeMesh::initCollapseCandidates()
{
    m_costs.resize(m_edges.size());

    for (size_t i = 0; i < m_edges.size(); ++i)
    {
        if (!isValidCollapseCandidate(EdgeID(i)))
            continue;

        uint32_t cost = computeCost(EdgeID(i));

        m_costs[i] = cost;
        m_sortedEdgeCollapseCandidates.insert(EdgeCollapseCandidate(EdgeID(i), cost));
    }
}

EdgeID ReducibleDirectedEdgeMesh::reduce()
{
    EdgeCollapseCandidate candidate;

    // It's possible that an edge isn't a valid candidate after collapse anymore
    // -> Get the first valid
    while (m_sortedEdgeCollapseCandidates.size() > 0)
    {
        candidate = *m_sortedEdgeCollapseCandidates.begin();
        m_sortedEdgeCollapseCandidates.erase(m_sortedEdgeCollapseCandidates.begin());

        if (isValidCollapseCandidate(candidate.edgeIdx))
            break;
    }

    if (m_sortedEdgeCollapseCandidates.size() == 0)
        return -1;

    assert(!m_removedFaces[candidate.edgeIdx / 3]);

    // Get all emanating edges of the neighbors of the vertex on the edge that was deleted
    // including its own emanating edges
    auto neighbors = getNeighbors(m_edges[candidate.edgeIdx].vertexIdx);
    neighbors.push_back(m_edges[candidate.edgeIdx].vertexIdx);
    std::vector<EdgeID> emanatingEdges;

    for (auto n : neighbors)
    {
        auto e = getEmanatingEdges(n);
        emanatingEdges.insert(emanatingEdges.end(), e.begin(), e.end());
    }

    collapse(candidate.edgeIdx);

    // Reevaluate the edges
    for (auto e : emanatingEdges)
    {
        reevaluate(e);

        if (m_edges[e].opposite >= 0)
            reevaluate(m_edges[e].opposite);
    }

    return candidate.edgeIdx;
}

void ReducibleDirectedEdgeMesh::reevaluate(EdgeID edgeIdx)
{
    m_sortedEdgeCollapseCandidates.erase(EdgeCollapseCandidate(edgeIdx, m_costs[edgeIdx]));

    if (m_removedFaces[edgeIdx / 3] || !isValidCollapseCandidate(edgeIdx))
        return;

    uint32_t cost = computeCost(edgeIdx);
    m_costs[edgeIdx] = cost;
    m_sortedEdgeCollapseCandidates.insert(EdgeCollapseCandidate(edgeIdx, cost));
}

bool ReducibleDirectedEdgeMesh::isValidCollapseCandidate(EdgeID edgeIdx)
{
    assert(edgeIdx >= 0 && edgeIdx < EdgeID(m_edges.size()) && !m_removedFaces[edgeIdx / 3]);

    auto eiID = edgeIdx;
    auto ejID = next(edgeIdx);

    auto& ei = m_edges[eiID];
    auto& ej = m_edges[ejID];

    auto& pi = m_vertices[ei.vertexIdx];
    auto& pj = m_vertices[ej.vertexIdx];

    EdgeID opposite = m_edges[edgeIdx].opposite;

    // If Pi and Pj are both boundary points then edge (Pi, Pj) must be a boundary edge
    if (pi.id < 0 && pj.id < 0)
    {
        if (opposite >= 0)
            return false;
    }

    std::vector<VertexIndex> adj;
    adj.push_back(m_edges[next(ejID)].vertexIdx);

    if (opposite >= 0)
        adj.push_back(m_edges[prev(opposite)].vertexIdx);

    std::sort(adj.begin(), adj.end());

    // For all points Pk adjacent to the points of the edge (Pi, Pj) a triangle (Pk, Pi, Pj) must exist
    // or in other words: the intersection between the sets of adjacent vertices of Pi and Pj
    // must be the set of the vertices opposite to the collapsed edge.
    auto adjPi = getNeighbors(ei.vertexIdx);
    auto adjPj = getNeighbors(ej.vertexIdx);
    auto adjOfBoth = setOp::sort::intersection(adjPi, adjPj);

    assert(adjOfBoth.size() != 0);

    if (!setOp::sorted::equal(adjOfBoth, adj))
        return false;

    // The valence of the points Pk mentioned above must be greater than 3.
    for (size_t i = 0; i < adjOfBoth.size(); ++i)
        if (valenceOf(adjOfBoth[i]) <= 3)
            return false;

    return true;
}

void ReducibleDirectedEdgeMesh::adjustOpposites(EdgeID edgeIdx)
{
    auto oppositeOfNext = m_edges[next(edgeIdx)].opposite;
    auto oppositeOfPrev = m_edges[prev(edgeIdx)].opposite;

    if (oppositeOfNext >= 0)
        m_edges[oppositeOfNext].opposite = oppositeOfPrev;

    if (oppositeOfPrev >= 0)
        m_edges[oppositeOfPrev].opposite = oppositeOfNext;
}

void ReducibleDirectedEdgeMesh::deleteEmanatingEdges(VertexIndex vIdx)
{
    auto vID = m_vertices[vIdx].id;
    if (vID < 0)
    {
        auto& e = m_emanatingEdges[-vID - 1];
        std::vector<size_t> erasePositions;
        for (int i = e.size() - 1; i >= 0; --i)
        {
            if (m_removedFaces[e[i] / 3])
                erasePositions.push_back(i);
        }

        assert(erasePositions.size() > 0);
        for (auto pos : erasePositions)
            e.erase(e.begin() + pos);
    }
}

void ReducibleDirectedEdgeMesh::adjustEmanatingEdgeIndex(VertexIndex vIdx)
{
    auto vID = m_vertices[vIdx].id;
    if (vID < 0)
    {
        m_vertices[vIdx].edgeID = m_emanatingEdges[-vID - 1][0];
        return;
    }    

    EdgeID startIdx = m_vertices[vIdx].edgeID;
    EdgeID curIndex = startIdx;

    do
    {
        assert(m_edges[curIndex].opposite >= 0);
        if (!m_removedFaces[curIndex / 3])
        {
            m_vertices[vIdx].edgeID = curIndex;
            return;
        }

        curIndex = next(m_edges[curIndex].opposite);
    } while (curIndex != startIdx);

    // This should never happen
    assert(false);
}

uint32_t ReducibleDirectedEdgeMesh::computeCost(EdgeID edgeIdx)
{
    auto vi0 = m_edges[edgeIdx].vertexIdx;
    auto vi1 = m_edges[next(edgeIdx)].vertexIdx;

    auto v0 = m_subMesh.vertices[vi0];
    auto v1 = m_subMesh.vertices[vi1];

    float edgeLength = glm::length(v0 - v1);
    float curvature = 0.0f;

    auto adjFacesStart = getAdjacentFaces(vi0);
    auto adjFacesNext = getAdjacentFaces(vi1);
    auto adjFacesEdge = setOp::intersection(adjFacesStart, adjFacesNext);

    for (size_t i = 0; i < adjFacesStart.size(); ++i)
    {
        float minCurvature = 1.0f;

        for (size_t j = 0; j < adjFacesEdge.size(); ++j)
        {
            auto n0 = computeFaceNormal(adjFacesStart[i]);
            auto n1 = computeFaceNormal(adjFacesEdge[j]);
            auto d = glm::dot(n0, n1);
            minCurvature = std::min(minCurvature, (1.0f - d) / 2.0f);
        }

        curvature = std::max(curvature, minCurvature);
    }

    // Convert float to an integer type to avoid floating point imprecision errors which fail the equality test on specific architectures.
    return uint32_t(edgeLength * curvature * 10e7f);
}

// Note: The vertex associated with ei is deleted.
void ReducibleDirectedEdgeMesh::collapse(EdgeID ei)
{
    assert(isValidCollapseCandidate(ei));

    EdgeID ej = next(ei);
    EdgeID opposite = m_edges[ei].opposite;

    auto vIdx = m_edges[ei].vertexIdx;
    auto emanatingEdges = getEmanatingEdges(vIdx);
    auto emanatingEdgesOfNext = getEmanatingEdges(m_edges[ej].vertexIdx);

    // Save the vertices opposite to the edge
    std::vector<VertexIndex> oppositeVertices;
    oppositeVertices.push_back(m_edges[next(ej)].vertexIdx);

    if (opposite >= 0)
        oppositeVertices.push_back(m_edges[prev(opposite)].vertexIdx);

    // Mark the faces as removed
    m_removedFaces[ei / 3] = true;
    ++m_removedFaceCount;
    if (opposite >= 0)
    {
        assert(m_edges[opposite].vertexIdx == m_edges[ej].vertexIdx);
        m_removedFaces[opposite / 3] = true;
        ++m_removedFaceCount;
    }

    // Delete the emanating edges which belong to the removed faces of border vertices 
    deleteEmanatingEdges(m_edges[ej].vertexIdx);
    for (auto i : oppositeVertices)
        deleteEmanatingEdges(i);

    adjustEmanatingEdgeIndex(m_edges[ej].vertexIdx);
    for (size_t i = 0; i < oppositeVertices.size(); ++i)
        adjustEmanatingEdgeIndex(oppositeVertices[i]);

    auto vID = m_vertices[m_edges[ej].vertexIdx].id;

    // Let neighbors of the deleted vertex point to its next vertex
    bool border = m_vertices[m_edges[ei].vertexIdx].id < 0;
    for (auto i : emanatingEdges)
    {
        if (m_removedFaces[i / 3])
            continue;

        m_edges[i].vertexIdx = m_edges[ej].vertexIdx;
    }

    adjustOpposites(ei);

    if (opposite >= 0)
        adjustOpposites(opposite);

    // Check if j turned into a border vertex
    if (border && vID >= 0)
    {
        vID = -VertexID(m_emanatingEdges.size()) - 1;
        m_vertices[m_edges[ej].vertexIdx].id = vID;
        m_emanatingEdges.push_back(emanatingEdgesOfNext);
        deleteEmanatingEdges(m_edges[ej].vertexIdx);
    }

    if (vID < 0)
    {
        // Add new emanating edges to the border vertex
        for (auto i : emanatingEdges)
        {
            if (m_removedFaces[i / 3])
                continue;

            m_emanatingEdges[-vID - 1].push_back(i);
        }
    }
}

Mesh::SubMesh ReducibleDirectedEdgeMesh::getReducedSubMesh()
{
    Mesh::SubMesh reducedMesh;
    std::vector<VertexID> vertexIDs;
    vertexIDs.resize(m_subMesh.vertices.size());
    std::fill(vertexIDs.begin(), vertexIDs.end(), -1);

    for (size_t i = 0; i < m_edges.size(); ++i)
    {
        auto faceIdx = i / 3;

        if (!m_removedFaces[faceIdx])
        {
            auto vIdx = m_edges[i].vertexIdx;
            if (vertexIDs[vIdx] < 0)
            {
                vertexIDs[vIdx] = reducedMesh.vertices.size();
                reducedMesh.vertices.push_back(m_subMesh.vertices[vIdx]);
                reducedMesh.normals.push_back(computeVertexNormal(vIdx));
            }

            reducedMesh.indices.push_back(vertexIDs[vIdx]);
        }
    }

    return reducedMesh;
}
