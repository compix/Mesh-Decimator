#pragma once
#include <vector>
#include "DirectedEdgeMesh.h"
#include <engine/util/math.h>

struct EdgeCollapseCandidate
{
    struct Compare
    {
        bool operator()(const EdgeCollapseCandidate& lhs, const EdgeCollapseCandidate& rhs) const
        {
            // It's important here to sort the values by cost and the edgeIdx to make them unique 
            // and thus avoid erasing multiple elements.
            if (lhs.cost != rhs.cost)
                return lhs.cost < rhs.cost;

            return lhs.edgeIdx < rhs.edgeIdx;
        }
    };

    EdgeCollapseCandidate() {}
    EdgeCollapseCandidate(EdgeID edgeIdx, uint32_t cost)
        : edgeIdx(edgeIdx), cost(cost) {}

    EdgeID edgeIdx{ INVALID_EDGE_ID };
    uint32_t cost{ 0 };
};

using EdgeCollapseCandidateContainer = std::set<EdgeCollapseCandidate, EdgeCollapseCandidate::Compare>;

class ReducibleDirectedEdgeMesh : public DirectedEdgeMesh
{
public:
    explicit ReducibleDirectedEdgeMesh(const Mesh::SubMesh& subMesh);
    ReducibleDirectedEdgeMesh() {}

    /**
    * The cost of a halfedge collapse is computed with the formula from the paper
    * "A Simple, Fast, and Effective Polygon Reduction Algorithm" by Stan Melax
    */
    uint32_t computeCost(EdgeID edgeIdx);

    /**
    * Collapses the given edge.
    * 1 vertex, 3 edges and 2 faces are removed if the operation is successful.
    * The given edgeIdx must be a valid collapse candidate.
    */
    void collapse(EdgeID edgeIdx);

    bool isValidCollapseCandidate(EdgeID edgeIdx);
    bool isFaceRemoved(FaceIndex faceIdx) { return m_removedFaces[faceIdx]; }
    bool reachedMaxReduction() const { return m_sortedEdgeCollapseCandidates.size() == 0; }
    size_t getFaceCount() const { return m_removedFaces.size() - m_removedFaceCount; }

    /**
    * Reduces the mesh by collapsing the lowest cost edge.
    * 1 vertex, 3 edges and 2 faces are removed if the operation is successful.
    * Returns the collapsed EdgeID on success otherwise the mesh can not further be reduced and -1 is returned.
    */
    EdgeID reduce();

    Mesh::SubMesh getReducedSubMesh();
private:
    // Fills the sorted candidate data structure.
    void initCollapseCandidates();
    void deleteEmanatingEdges(VertexIndex vIdx);
    // The data structure stores one emanating edge index per vertex. 
    // During an edge collapse edges are removed however, thus the emanating edge of affected vertices
    // might need to be changed.
    void adjustEmanatingEdgeIndex(VertexIndex vIdx);
    // The cached opposite edge ids of edges that are opposite to removed edges need to be adjusted.
    void adjustOpposites(EdgeID edgeIdx);

    void reevaluate(EdgeID edgeIdx);
private:
    // Faces are just marked as removed for O(1) removal. Vertices and edges still remain in the structure.
    std::vector<bool> m_removedFaces;
    size_t m_removedFaceCount{ 0 };
    // Cache costs to allow log(n) retrieval of collapse candidates.
    // The index into the vector corresponds to the EdgeID.
    std::vector<uint32_t> m_costs;
    // Data structure for the edge collapse candidates.
    // Properties: O(1) access to min cost candidate, log(n) to erase and add a candidate
    // A max mesh decimation is in O(n * log(n)) where n is the number of halfedges
    EdgeCollapseCandidateContainer m_sortedEdgeCollapseCandidates;
};
