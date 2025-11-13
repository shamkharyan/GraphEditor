#ifndef GRAPH_H
#define GRAPH_H

#include "Vertex.h"
#include "Edge.h"

#include <unordered_map>
#include <unordered_set>
#include <string>

class Graph
{
public:
    using VertexMap = std::unordered_map<int, Vertex>;
    using EdgeMap = std::unordered_map<int, Edge>;
    using ConnectedEdgesMap = std::unordered_map<int, std::unordered_set<int>>;

    int addVertex(float x, float y, const std::string& name = "");
    int addVertexWithID(int id, float x, float y, const std::string& name = "");
    void removeVertex(int id);
    int addEdge(int startVertexId, int endVertexId, float weight, const std::string& name = "");
    int addEdgeWithID(int id, int startVertexId, int endVertexId, float weight, const std::string& name = "");
    void removeEdge(int id);

    // Getters
    const VertexMap& getVertices() const;
    const EdgeMap& getEdges() const;
    const ConnectedEdgesMap& getConnectedEdgesIds() const;

private:

    // vertexId -> Vertex
    VertexMap m_vertices;

    // edgeId -> Edge
    EdgeMap m_edges;

    // vertexId -> set of connected edgeIds
    ConnectedEdgesMap m_connectedEdgesIds;

    int m_nextEdgeId = 0;
    int m_nextVertexId = 0;
};

#endif // GRAPH_H
