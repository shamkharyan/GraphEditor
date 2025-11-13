#include "algorithms/GraphAlgorithms.h"
#include "model/Graph.h"
#include "model/DisjointSet.h"

#include <vector>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>
#include <limits>

Graph GraphAlgorithms::buildKruskalMST(const Graph& graph)
{
    Graph mst;
    DisjointSet<int> set;

    for (const auto& [id, vertex] : graph.getVertices())
    {
        mst.addVertex(vertex.getX(), vertex.getY(), vertex.getName());
        set.makeSet(id);
    }

    const auto& edgesMap = graph.getEdges();
    std::vector<Edge> edges;
    edges.reserve(edgesMap.size());
    for (const auto& [id, edge] : edgesMap)
        edges.push_back(edge);

    std::sort(edges.begin(), edges.end(),
              [](const Edge& a, const Edge& b) {
                  return a.getWeight() < b.getWeight();
              });

    for (const Edge& edge : edges)
    {
        int startId = edge.getStartVertexID();
        int endId = edge.getEndVertexID();

        if (set.unionSets(startId, endId))
            mst.addEdge(startId, endId, edge.getWeight(), edge.getName());
    }

    if (mst.getEdges().size() < graph.getVertices().size() - 1)
        throw std::runtime_error("Graph is disconnected: MST does not exist for the entire graph.");

    return mst;
}

Graph GraphAlgorithms::buildPrimMST(const Graph& graph)
{
    Graph mst;

    const auto& vertices = graph.getVertices();
    const auto& edges = graph.getEdges();
    const auto& connectedEdges = graph.getConnectedEdgesIds();

    std::unordered_map<int, float> cheapestCost;
    std::unordered_map<int, int> cheapestEdge;
    std::unordered_set<int> unexploredVertices;

    for (const auto& [id, vertex] : vertices)
    {
        mst.addVertex(vertex.getX(), vertex.getY(), vertex.getName());
        cheapestCost[id] = std::numeric_limits<float>::max();
        cheapestEdge[id] = -1;
        unexploredVertices.insert(id);
    }

    if (vertices.empty())
        return mst;

    int startVertex = vertices.begin()->first;
    cheapestCost[startVertex] = 0;

    while (!unexploredVertices.empty())
    {
        int currentVertex = -1;
        float minCost = std::numeric_limits<float>::max();
        for (int v : unexploredVertices)
        {
            if (cheapestCost[v] < minCost)
            {
                minCost = cheapestCost[v];
                currentVertex = v;
            }
        }

        if (currentVertex == -1)
            break;

        unexploredVertices.erase(currentVertex);

        auto it = connectedEdges.find(currentVertex);
        if (it == connectedEdges.end())
            continue;

        for (int edgeId : it->second)
        {
            const auto& edge = edges.at(edgeId);
            int neighbor = (edge.getStartVertexID() == currentVertex)
                               ? edge.getEndVertexID()
                               : edge.getStartVertexID();

            if (unexploredVertices.count(neighbor) &&
                edge.getWeight() < cheapestCost[neighbor])
            {
                cheapestCost[neighbor] = edge.getWeight();
                cheapestEdge[neighbor] = edgeId;
            }
        }
    }

    for (const auto& [id, vertex] : vertices)
    {
        if (cheapestEdge[id] != -1)
        {
            const auto& edge = edges.at(cheapestEdge[id]);
            mst.addEdge(edge.getStartVertexID(),
                        edge.getEndVertexID(),
                        edge.getWeight(),
                        edge.getName());
        }
    }

    if (mst.getEdges().size() < vertices.size() - 1)
        throw std::runtime_error("Graph is disconnected: MST does not exist for the entire graph.");

    return mst;
}

