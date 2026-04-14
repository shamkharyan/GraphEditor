#include "GraphGenerator.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <numeric>
#include <stdexcept>
#include <unordered_set>
#include <vector>

namespace
{
using EdgeKey = std::uint64_t;

EdgeKey makeEdgeKey(int a, int b)
{
    if (a > b)
        std::swap(a, b);

    return (static_cast<EdgeKey>(static_cast<std::uint32_t>(a)) << 32U) |
           static_cast<std::uint32_t>(b);
}

long long maxSimpleEdges(int vertexCount)
{
    return 1LL * vertexCount * (vertexCount - 1) / 2;
}
} // namespace

GraphGenerator::GraphGenerator(const GraphGeneratorConfig& config)
    : m_config(config)
    , m_rng(config.seed)
{
    if (m_config.graphCount <= 0)
        throw std::invalid_argument("graphCount must be positive");
    if (m_config.minVertices < 2)
        throw std::invalid_argument("minVertices must be >= 2");
    if (m_config.maxVertices < m_config.minVertices)
        throw std::invalid_argument("maxVertices must be >= minVertices");
    if (m_config.minDensity <= 0.0 || m_config.maxDensity <= 0.0)
        throw std::invalid_argument("Density bounds must be positive");
    if (m_config.maxDensity < m_config.minDensity)
        throw std::invalid_argument("maxDensity must be >= minDensity");
}

GraphSampleSpec GraphGenerator::nextSpec()
{
    return sampleSpec();
}

Graph GraphGenerator::makeGraph(const GraphSampleSpec& spec)
{
    return makeConnectedGraph(spec.n, spec.m, m_rng);
}

Graph GraphGenerator::makeConnectedGraph(int vertexCount, int edgeCount, std::mt19937& rng)
{
    if (edgeCount < vertexCount - 1)
        throw std::invalid_argument("edgeCount must be >= vertexCount - 1 for a connected graph");

    const long long maxEdges = maxSimpleEdges(vertexCount);
    if (edgeCount > maxEdges)
        throw std::invalid_argument("edgeCount exceeds maximum for a simple graph");

    Graph graph;
    std::uniform_real_distribution<float> posDist(-1000.0f, 1000.0f);
    std::uniform_real_distribution<float> weightDist(0.01f, 100.0f);

    for (int i = 0; i < vertexCount; ++i)
        graph.addVertex(posDist(rng), posDist(rng));

    std::unordered_set<EdgeKey> used;
    used.reserve(static_cast<std::size_t>(edgeCount * 1.3));

    std::vector<int> permutation(vertexCount);
    std::iota(permutation.begin(), permutation.end(), 0);
    std::shuffle(permutation.begin(), permutation.end(), rng);

    for (int i = 1; i < vertexCount; ++i)
    {
        std::uniform_int_distribution<int> parentDist(0, i - 1);
        const int a = permutation[i];
        const int b = permutation[parentDist(rng)];
        used.insert(makeEdgeKey(a, b));
        graph.addEdge(a, b, weightDist(rng));
    }

    if (edgeCount == vertexCount - 1)
        return graph;

    const long long totalPairs = maxEdges;
    const long long targetEdges = edgeCount;
    const bool buildDenseDirectly = (targetEdges * 10LL >= totalPairs * 6LL);

    if (!buildDenseDirectly)
    {
        std::uniform_int_distribution<int> vertexDist(0, vertexCount - 1);

        while (static_cast<int>(graph.getEdges().size()) < edgeCount)
        {
            int a = vertexDist(rng);
            int b = vertexDist(rng);
            if (a == b)
                continue;

            const EdgeKey key = makeEdgeKey(a, b);
            if (used.insert(key).second)
                graph.addEdge(a, b, weightDist(rng));
        }

        return graph;
    }

    const long long missingEdgeCount = totalPairs - targetEdges;
    std::unordered_set<EdgeKey> missing;
    missing.reserve(static_cast<std::size_t>(missingEdgeCount * 1.3 + 8));

    std::uniform_int_distribution<int> vertexDist(0, vertexCount - 1);
    while (static_cast<long long>(missing.size()) < missingEdgeCount)
    {
        int a = vertexDist(rng);
        int b = vertexDist(rng);
        if (a == b)
            continue;

        const EdgeKey key = makeEdgeKey(a, b);
        if (used.find(key) != used.end())
            continue;

        missing.insert(key);
    }

    Graph denseGraph;
    for (const auto& [id, vertex] : graph.getVertices())
        denseGraph.addVertexWithID(id, vertex.getX(), vertex.getY(), vertex.getName());

    for (int a = 0; a < vertexCount; ++a)
    {
        for (int b = a + 1; b < vertexCount; ++b)
        {
            const EdgeKey key = makeEdgeKey(a, b);
            if (missing.find(key) == missing.end())
                denseGraph.addEdge(a, b, weightDist(rng));
        }
    }

    return denseGraph;
}

GraphSampleSpec GraphGenerator::sampleSpec()
{
    const double density = sampleLogUniform(m_config.minDensity, m_config.maxDensity);
    const int vertexCount = sampleVertexCount(density);

    const long long maxEdges = maxSimpleEdges(vertexCount);
    const long long desiredEdges = static_cast<long long>(
        std::llround(density * static_cast<double>(maxEdges)));

    const long long boundedEdges = std::clamp<long long>(
        desiredEdges,
        vertexCount - 1,
        std::min<long long>(maxEdges, static_cast<long long>(m_config.maxEdgesPerGraph)));

    GraphSampleSpec spec;
    spec.n = vertexCount;
    spec.m = static_cast<int>(boundedEdges);
    spec.density = (maxEdges > 0)
        ? (2.0 * static_cast<double>(spec.m)) /
          (static_cast<double>(spec.n) * static_cast<double>(spec.n - 1))
        : 0.0;
    return spec;
}

int GraphGenerator::sampleVertexCount(double density)
{
    const double edgeBudget = static_cast<double>(m_config.maxEdgesPerGraph);
    const double discriminant = 1.0 + 8.0 * edgeBudget / density;
    const double budgetLimitedMax = (1.0 + std::sqrt(discriminant)) * 0.5;

    int maxAllowed = std::min(
        m_config.maxVertices,
        static_cast<int>(std::floor(budgetLimitedMax)));

    maxAllowed = std::max(maxAllowed, m_config.minVertices);

    const double sampled = sampleLogUniform(
        static_cast<double>(m_config.minVertices),
        static_cast<double>(maxAllowed));

    return std::clamp(
        static_cast<int>(std::llround(sampled)),
        m_config.minVertices,
        maxAllowed);
}

double GraphGenerator::sampleLogUniform(double minValue, double maxValue)
{
    if (minValue == maxValue)
        return minValue;

    std::uniform_real_distribution<double> dist(std::log(minValue), std::log(maxValue));
    return std::exp(dist(m_rng));
}
