#ifndef EUCLIDEAN_GRAPH_H
#define EUCLIDEAN_GRAPH_H

#include "Vertex.h"
#include "Edge.h"

#include <unordered_map>
#include <string>

class EuclideanGraph
{
public:
    void addVertex(float x, float y, const std::string& name = "");
    void removeVertex(int id);
    void addEdge(int startVertexId, int endVertexId, const std::string& name = "");
    void removeEdge(int id);

private:

    // vertexId -> Vertex
    std::unordered_map<int, Vertex> m_verticies;

    // edgeId -> Edge
    std::unordered_map<int, Edge> m_edges;

    // vertexId -> connected Edges id
    std::unordered_map<int, std::unordered_map<int, int>> m_connectedEdges;

    // vertexId -> connected Verticies id
    // std::unordered_map<int, std::unordered_map<int, int>> m_connectedVerticies;

    // 0 -> {01}
    // 1 -> {01,12}
    // 2 -> {12}
    // 3 -> {}
};

#endif // EUCLIDEAN_GRAPH_H
