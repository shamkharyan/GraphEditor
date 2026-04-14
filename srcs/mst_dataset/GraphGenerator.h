#ifndef MST_DATASET_GRAPH_GENERATOR_H
#define MST_DATASET_GRAPH_GENERATOR_H

#include "model/Graph.h"

#include <cstddef>
#include <cstdint>
#include <random>

struct GraphSampleSpec
{
    int n = 0;
    int m = 0;
    double density = 0.0;
};

struct GraphGeneratorConfig
{
    int graphCount = 10000;
    int minVertices = 300;
    int maxVertices = 15000;
    double minDensity = 0.001;
    double maxDensity = 0.85;
    std::size_t maxEdgesPerGraph = 200000;
    std::uint32_t seed = 42;
};

class GraphGenerator
{
public:
    explicit GraphGenerator(const GraphGeneratorConfig& config);

    GraphSampleSpec nextSpec();
    Graph makeGraph(const GraphSampleSpec& spec);

    static Graph makeConnectedGraph(int vertexCount, int edgeCount, std::mt19937& rng);

private:
    GraphSampleSpec sampleSpec();
    int sampleVertexCount(double density);
    double sampleLogUniform(double minValue, double maxValue);

    GraphGeneratorConfig m_config;
    std::mt19937 m_rng;
};

#endif // MST_DATASET_GRAPH_GENERATOR_H
