#include "GraphGenerator.h"
#include "FeatureExtractor.h"
#include "algorithms/GraphAlgorithms.h"

#include <mlpack/core.hpp>
#include <mlpack/methods/random_forest/random_forest.hpp>

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

namespace
{
constexpr std::size_t kFeatureCount = 12;
constexpr std::size_t kKruskalLabel = 0;
constexpr std::size_t kPrimLabel = 1;

arma::mat toFeatureVector(const GraphFeatures& features)
{
    arma::mat sample(kFeatureCount, 1, arma::fill::zeros);
    sample(0, 0) = static_cast<double>(features.n);
    sample(1, 0) = static_cast<double>(features.m);
    sample(2, 0) = features.density;
    sample(3, 0) = features.avgDegree;
    sample(4, 0) = features.logN;
    sample(5, 0) = features.logM;
    sample(6, 0) = features.meanWeight;
    sample(7, 0) = features.stdWeight;
    sample(8, 0) = features.minWeight;
    sample(9, 0) = features.maxWeight;
    sample(10, 0) = features.skewWeight;
    sample(11, 0) = static_cast<double>(features.connectedComponents);
    return sample;
}

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

const char* labelToName(std::size_t label)
{
    return (label == kPrimLabel) ? "prim" : "kruskal";
}

int parseGraphCount(int argc, char** argv)
{
    if (argc < 2)
        return 2000;

    const int value = std::stoi(argv[1]);
    if (value <= 0)
        throw std::invalid_argument("Validation graph count must be positive");
    return value;
}

std::string parseOutputPath(int argc, char** argv)
{
    if (argc >= 3)
        return argv[2];
    return "validation_results.csv";
}

std::string parseModelPath(int argc, char** argv)
{
    if (argc >= 4)
        return argv[3];
    return "mst_rf_model.bin";
}
} // namespace

int main(int argc, char** argv)
{
    try
    {
        constexpr int runsPerAlgorithm = 5;
        constexpr int progressInterval = 200;

        const int graphCount = parseGraphCount(argc, argv);
        const std::string outputPath = parseOutputPath(argc, argv);
        const std::string modelPath = parseModelPath(argc, argv);

        mlpack::RandomForest<> forest;
        if (!mlpack::data::Load(modelPath, "mst_random_forest", forest, true))
            throw std::runtime_error("Cannot load model from " + modelPath);

        GraphGeneratorConfig config;
        config.graphCount = graphCount;
        config.seed = 1337;
        config.minVertices = 300;
        config.maxVertices = 15000;
        config.minDensity = 0.001;
        config.maxDensity = 0.85;
        config.maxEdgesPerGraph = 200000;

        GraphGenerator generator(config);

        std::ofstream csv(outputPath);
        if (!csv)
            throw std::runtime_error("Cannot open output CSV: " + outputPath);

        csv << "n,m,density,avg_degree,log_n,log_m,mean_weight,std_weight,min_weight,max_weight,skew_weight,connected_components,kruskal_ms,prim_ms,auto_predicted,best_actual,is_correct,auto_time_ms,time_gain_vs_kruskal_ms,time_gain_vs_prim_ms,time_regret_ms\n";
        csv << std::fixed << std::setprecision(8);

        int correctPredictions = 0;
        double totalAutoTime = 0.0;
        double totalKruskalTime = 0.0;
        double totalPrimTime = 0.0;
        double totalRegret = 0.0;

        std::cout << "Running validation: graphs=" << graphCount
                  << ", model=" << modelPath
                  << ", seed=" << config.seed << "\n";

        for (int i = 0; i < graphCount; ++i)
        {
            const GraphSampleSpec spec = generator.nextSpec();
            Graph graph = generator.makeGraph(spec);
            const GraphFeatures features = FeatureExtractor::extract(graph);

            const double kruskalMs = timeMedian(graph, GraphAlgorithms::buildKruskalMST, runsPerAlgorithm);
            const double primMs = timeMedian(graph, GraphAlgorithms::buildPrimMST, runsPerAlgorithm);

            arma::Row<std::size_t> prediction;
            forest.Classify(toFeatureVector(features), prediction);
            const std::size_t predictedLabel = prediction[0];
            const std::size_t actualBestLabel = (kruskalMs <= primMs) ? kKruskalLabel : kPrimLabel;
            const bool isCorrect = (predictedLabel == actualBestLabel);
            const double bestTimeMs = std::min(kruskalMs, primMs);
            const double autoTimeMs = (predictedLabel == kPrimLabel) ? primMs : kruskalMs;
            const double regretMs = autoTimeMs - bestTimeMs;

            correctPredictions += isCorrect ? 1 : 0;
            totalAutoTime += autoTimeMs;
            totalKruskalTime += kruskalMs;
            totalPrimTime += primMs;
            totalRegret += regretMs;

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
                << kruskalMs << ','
                << primMs << ','
                << labelToName(predictedLabel) << ','
                << labelToName(actualBestLabel) << ','
                << (isCorrect ? 1 : 0) << ','
                << autoTimeMs << ','
                << (kruskalMs - autoTimeMs) << ','
                << (primMs - autoTimeMs) << ','
                << regretMs << '\n';

            if ((i + 1) % progressInterval == 0 || i + 1 == graphCount)
            {
                const double runningAccuracy = 100.0 * static_cast<double>(correctPredictions) / static_cast<double>(i + 1);
                std::cout << '[' << (i + 1) << '/' << graphCount << "] accuracy="
                          << std::setprecision(4) << runningAccuracy << "%"
                          << ", avg_regret_ms=" << (totalRegret / static_cast<double>(i + 1))
                          << "\n";
            }
        }

        const double accuracy = 100.0 * static_cast<double>(correctPredictions) / static_cast<double>(graphCount);
        std::cout << std::fixed << std::setprecision(4);
        std::cout << "Validation saved to " << outputPath << "\n";
        std::cout << "Accuracy: " << accuracy << "%\n";
        std::cout << "Avg auto time: " << (totalAutoTime / graphCount) << " ms\n";
        std::cout << "Avg kruskal time: " << (totalKruskalTime / graphCount) << " ms\n";
        std::cout << "Avg prim time: " << (totalPrimTime / graphCount) << " ms\n";
        std::cout << "Avg regret: " << (totalRegret / graphCount) << " ms\n";
        return 0;
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Validation failed: " << ex.what() << "\n";
        return 1;
    }
}
