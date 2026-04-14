// Standalone MST dataset generator for ML-based heuristic selection.
//
// Build (from project root, Release):
// g++ -O3 -DNDEBUG -std=c++17 -Iincludes \
//     srcs/mst_dataset/main.cpp \
//     srcs/mst_dataset/GraphGenerator.cpp \
//     srcs/mst_dataset/FeatureExtractor.cpp \
//     srcs/model/Graph.cpp srcs/model/Vertex.cpp srcs/model/Edge.cpp \
//     srcs/algorithms/GraphAlgorithms.cpp \
//     -o build/mst_dataset_generator

#include "GraphGenerator.h"
#include "FeatureExtractor.h"
#include "algorithms/GraphAlgorithms.h"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using Clock = std::chrono::high_resolution_clock;
using Ms = std::chrono::duration<double, std::milli>;

template<typename Algo>
double timeMedian(const Graph& graph, Algo algo, int runs = 5)
{
    std::vector<double> times;
    times.reserve(runs);

    for (int i = 0; i < runs; ++i)
    {
        const auto t0 = Clock::now();
        algo(graph);
        const auto t1 = Clock::now();
        times.push_back(Ms(t1 - t0).count());
    }

    std::sort(times.begin(), times.end());
    return times[runs / 2];
}

int parseGraphCount(int argc, char** argv)
{
    if (argc < 2)
        return 10000;

    const int value = std::stoi(argv[1]);
    if (value < 8000 || value > 15000)
        throw std::invalid_argument("Graph count must be in [8000, 15000]");
    return value;
}

std::string parseOutputPath(int argc, char** argv)
{
    if (argc >= 3)
        return argv[2];
    return "mst_dataset.csv";
}

int main(int argc, char** argv)
{
    try
    {
        constexpr int runsPerAlgorithm = 5;
        constexpr int progressInterval = 400;

        GraphGeneratorConfig config;
        config.graphCount = parseGraphCount(argc, argv);
        config.seed = 42;
        config.minVertices = 300;
        config.maxVertices = 15000;
        config.minDensity = 0.001;
        config.maxDensity = 0.85;
        config.maxEdgesPerGraph = 200000;

        const std::string outputPath = parseOutputPath(argc, argv);

        GraphGenerator generator(config);

        std::ofstream csv(outputPath);
        if (!csv)
        {
            std::cerr << "Cannot open " << outputPath << " for writing.\n";
            return 1;
        }

        csv << "n,m,density,avg_degree,log_n,log_m,mean_weight,std_weight,min_weight,max_weight,skew_weight,connected_components,best_algorithm,best_time_ms\n";
        csv << std::fixed << std::setprecision(8);

        std::cout << "Generating MST dataset: graphs=" << config.graphCount
                  << ", seed=" << config.seed
                  << ", n in [" << config.minVertices << ", " << config.maxVertices << "]"
                  << ", density in [" << config.minDensity << ", " << config.maxDensity << "]\n";
        std::cout << "Edge budget per graph: " << config.maxEdgesPerGraph
                  << " (keeps dense samples practical while preserving the requested range)\n";

        for (int i = 0; i < config.graphCount; ++i)
        {
            const GraphSampleSpec spec = generator.nextSpec();
            Graph graph = generator.makeGraph(spec);

            const GraphFeatures features = FeatureExtractor::extract(graph);
            const double kruskalMs = timeMedian(graph, GraphAlgorithms::buildKruskalMST, runsPerAlgorithm);
            const double primMs = timeMedian(graph, GraphAlgorithms::buildPrimMST, runsPerAlgorithm);

            const bool kruskalIsBetter = kruskalMs <= primMs;
            const char* bestAlgorithm = kruskalIsBetter ? "kruskal" : "prim";
            const double bestTimeMs = kruskalIsBetter ? kruskalMs : primMs;

            csv << features.n << ','
                << features.m << ','
                << features.density << ','
                << features.avgDegree << ','
                << features.logN << ','
                << features.logM << ','
                << features.meanWeight << ','
                << features.stdWeight << ','
                << features.minWeight << ','
                << features.maxWeight << ','
                << features.skewWeight << ','
                << features.connectedComponents << ','
                << bestAlgorithm << ','
                << bestTimeMs << '\n';

            if ((i + 1) % progressInterval == 0 || i + 1 == config.graphCount)
            {
                std::cout << "[" << (i + 1) << "/" << config.graphCount << "] "
                          << "n=" << features.n
                          << ", m=" << features.m
                          << ", best=" << bestAlgorithm
                          << ", best_time_ms=" << std::fixed << std::setprecision(4)
                          << bestTimeMs << "\n";
            }
        }

        std::cout << "Dataset saved to " << outputPath << "\n";
        return 0;
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Dataset generation failed: " << ex.what() << "\n";
        return 1;
    }
}
