#include "FeatureExtractor.h"

#include "model/Edge.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>
#include <unordered_set>
#include <vector>

namespace
{
int countConnectedComponents(const Graph& graph)
{
    const auto& vertices = graph.getVertices();
    const auto& edges = graph.getEdges();
    const auto& adjacency = graph.getConnectedEdgesIds();

    std::unordered_set<int> visited;
    visited.reserve(vertices.size());

    int componentCount = 0;
    std::queue<int> pending;

    for (const auto& [vertexId, vertex] : vertices)
    {
        (void)vertex;
        if (visited.find(vertexId) != visited.end())
            continue;

        ++componentCount;
        visited.insert(vertexId);
        pending.push(vertexId);

        while (!pending.empty())
        {
            const int current = pending.front();
            pending.pop();

            const auto adjacencyIt = adjacency.find(current);
            if (adjacencyIt == adjacency.end())
                continue;

            for (int edgeId : adjacencyIt->second)
            {
                const auto edgeIt = edges.find(edgeId);
                if (edgeIt == edges.end())
                    continue;

                const Edge& edge = edgeIt->second;
                const int next = (edge.getStartVertexID() == current)
                    ? edge.getEndVertexID()
                    : edge.getStartVertexID();

                if (visited.insert(next).second)
                    pending.push(next);
            }
        }
    }

    return componentCount;
}
} // namespace

GraphFeatures FeatureExtractor::extract(const Graph& graph)
{
    GraphFeatures features;

    const auto& vertices = graph.getVertices();
    const auto& edges = graph.getEdges();

    features.n = static_cast<int>(vertices.size());
    features.m = static_cast<int>(edges.size());
    features.density = (features.n > 1)
        ? (2.0 * static_cast<double>(features.m)) /
          (static_cast<double>(features.n) * static_cast<double>(features.n - 1))
        : 0.0;
    features.avgDegree = (features.n > 0)
        ? (2.0 * static_cast<double>(features.m)) / static_cast<double>(features.n)
        : 0.0;
    features.logN = (features.n > 0) ? std::log(static_cast<double>(features.n)) : 0.0;
    features.logM = (features.m > 0) ? std::log(static_cast<double>(features.m)) : 0.0;

    if (!edges.empty())
    {
        std::vector<double> weights;
        weights.reserve(edges.size());

        double sum = 0.0;
        features.minWeight = std::numeric_limits<double>::infinity();
        features.maxWeight = -std::numeric_limits<double>::infinity();

        for (const auto& [edgeId, edge] : edges)
        {
            (void)edgeId;
            const double weight = static_cast<double>(edge.getWeight());
            weights.push_back(weight);
            sum += weight;
            features.minWeight = std::min(features.minWeight, weight);
            features.maxWeight = std::max(features.maxWeight, weight);
        }

        features.meanWeight = sum / static_cast<double>(weights.size());

        double varianceAcc = 0.0;
        double skewAcc = 0.0;
        for (double weight : weights)
        {
            const double centered = weight - features.meanWeight;
            varianceAcc += centered * centered;
            skewAcc += centered * centered * centered;
        }

        const double variance = varianceAcc / static_cast<double>(weights.size());
        features.stdWeight = std::sqrt(std::max(0.0, variance));

        if (features.stdWeight > 1e-12)
        {
            const double m3 = skewAcc / static_cast<double>(weights.size());
            features.skewWeight = m3 / std::pow(features.stdWeight, 3.0);
        }
    }

    features.connectedComponents = countConnectedComponents(graph);
    return features;
}
