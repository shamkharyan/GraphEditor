#include "algorithms/MSTModelSelector.h"

#include "mst_dataset/FeatureExtractor.h"

#include <mlpack/core.hpp>
#include <mlpack/methods/random_forest/random_forest.hpp>

#include <armadillo>

#include <cmath>
#include <filesystem>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

namespace
{
constexpr std::size_t kFeatureCount = 12;
constexpr std::size_t kKruskalLabel = 0;
constexpr std::size_t kPrimLabel = 1;

class CachedModel
{
public:
    MSTModelSelector::Algorithm predict(const Graph& graph)
    {
        ensureLoaded();
        if (!m_loaded)
            return predictFallback(graph);

        const GraphFeatures features = FeatureExtractor::extract(graph);
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

        arma::Row<std::size_t> prediction;
        m_model.Classify(sample, prediction);
        return (prediction[0] == kPrimLabel)
            ? MSTModelSelector::Algorithm::Prim
            : MSTModelSelector::Algorithm::Kruskal;
    }

    bool loaded()
    {
        ensureLoaded();
        return m_loaded;
    }

    std::string status()
    {
        ensureLoaded();
        return m_status;
    }

private:
    void ensureLoaded()
    {
        std::call_once(m_once, [this]() { load(); });
    }

    void load()
    {
        const std::vector<std::filesystem::path> candidates = {
            std::filesystem::current_path() / "mst_rf_model.bin",
            std::filesystem::current_path() / ".." / "mst_rf_model.bin",
            std::filesystem::current_path() / ".." / ".." / "mst_rf_model.bin",
            std::filesystem::path("/home/pavel/GraphEditor/mst_rf_model.bin")
        };

        for (const auto& path : candidates)
        {
            std::error_code ec;
            if (!std::filesystem::exists(path, ec))
                continue;

            try
            {
                const std::string pathString = path.string();
                if (mlpack::data::Load(pathString, "mst_random_forest", m_model, true))
                {
                    m_loaded = true;
                    m_status = "Loaded model from " + pathString;
                    return;
                }
            }
            catch (const std::exception& ex)
            {
                m_status = "Model load failed from " + path.string() + ": " + ex.what();
            }
        }

        if (m_status.empty())
            m_status = "Model file mst_rf_model.bin not found; using heuristic fallback";
    }

    MSTModelSelector::Algorithm predictFallback(const Graph& graph) const
    {
        const std::size_t vertexCount = graph.getVertices().size();
        const std::size_t edgeCount = graph.getEdges().size();
        const auto logV = static_cast<std::size_t>(
            vertexCount > 1 ? std::log2(static_cast<double>(vertexCount)) : 1.0);

        return (edgeCount > vertexCount * logV)
            ? MSTModelSelector::Algorithm::Prim
            : MSTModelSelector::Algorithm::Kruskal;
    }

    std::once_flag m_once;
    bool m_loaded = false;
    std::string m_status;
    mlpack::RandomForest<> m_model;
};

CachedModel& model()
{
    static CachedModel cachedModel;
    return cachedModel;
}
} // namespace

MSTModelSelector::Algorithm MSTModelSelector::predict(const Graph& graph)
{
    return model().predict(graph);
}

bool MSTModelSelector::isModelAvailable()
{
    return model().loaded();
}

std::string MSTModelSelector::modelStatus()
{
    return model().status();
}

MSTModelSelector::Algorithm MSTModelSelector::predictWithHeuristic(const Graph& graph)
{
    const std::size_t vertexCount = graph.getVertices().size();
    const std::size_t edgeCount = graph.getEdges().size();
    const auto logV = static_cast<std::size_t>(
        vertexCount > 1 ? std::log2(static_cast<double>(vertexCount)) : 1.0);

    return (edgeCount > vertexCount * logV) ? Algorithm::Prim : Algorithm::Kruskal;
}
