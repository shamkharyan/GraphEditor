// Standalone MST benchmark — Kruskal vs Prim
// Sparse regime: E ≈ EDGE_FACTOR * V, sweep V from V_MIN to V_MAX
// Output: benchmark_results.csv  (vertices, edges, kruskal_ms, prim_ms)
//
// Build (from graph_editor root):
//   g++ -O2 -std=c++17 -I. benchmark_sparse.cpp model/Graph.cpp model/Vertex.cpp model/Edge.cpp \
//       algorithms/GraphAlgorithms.cpp -o benchmark_sparse

#include "model/Graph.h"
#include "model/Vertex.h"
#include "model/Edge.h"
#include "algorithms/GraphAlgorithms.h"

#include <chrono>
#include <random>
#include <vector>
#include <set>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <numeric>
#include <stdexcept>

using Clock = std::chrono::high_resolution_clock;
using Ms    = std::chrono::duration<double, std::milli>;

// ── Graph generator ─────────────────────────────────────────────────────────
Graph makeConnectedGraph(int V, int E, std::mt19937& rng)
{
    if (E < V - 1)
        throw std::invalid_argument("E must be >= V-1 for a connected graph");
    if (E > V * (V - 1) / 2)
        throw std::invalid_argument("E exceeds maximum for a simple graph");

    Graph g;
    std::uniform_real_distribution<float> posD(-1000.f, 1000.f);
    std::uniform_real_distribution<float> wD(0.01f, 100.f);

    for (int i = 0; i < V; ++i)
        g.addVertex(posD(rng), posD(rng));

    // Step 1: random spanning tree (guarantees connectivity)
    std::vector<int> perm(V);
    std::iota(perm.begin(), perm.end(), 0);
    std::shuffle(perm.begin(), perm.end(), rng);

    std::set<std::pair<int,int>> used;
    for (int i = 1; i < V; ++i)
    {
        std::uniform_int_distribution<int> pickD(0, i - 1);
        int a = perm[i];
        int b = perm[pickD(rng)];
        if (a > b) std::swap(a, b);
        used.insert({a, b});
        g.addEdge(a, b, wD(rng));
    }

    // Step 2: add extra edges (sparse → collisions are rare, no retry limit needed)
    std::uniform_int_distribution<int> vD(0, V - 1);
    while ((int)g.getEdges().size() < E)
    {
        int a = vD(rng), b = vD(rng);
        if (a == b) continue;
        if (a > b) std::swap(a, b);
        if (used.insert({a, b}).second)
            g.addEdge(a, b, wD(rng));
    }

    return g;
}

// ── Timer ───────────────────────────────────────────────────────────────────
template<typename Algo>
double timeMedian(const Graph& g, Algo algo, int runs = 5)
{
    std::vector<double> times;
    times.reserve(runs);
    for (int i = 0; i < runs; ++i)
    {
        auto t0 = Clock::now();
        try { algo(g); } catch (...) {}
        auto t1 = Clock::now();
        times.push_back(Ms(t1 - t0).count());
    }
    std::sort(times.begin(), times.end());
    return times[runs / 2];
}

// ── Main ────────────────────────────────────────────────────────────────────
int main()
{
    constexpr int RUNS         = 5;    // runs per data point (median taken)
    constexpr int POINTS       = 40;   // number of vertex-count samples

    constexpr int V_MIN        = 100;
    constexpr int V_MAX        = 40000;

    // E = EDGE_FACTOR * V  →  sparse (E ~ V) regime
    // Must be > 1 so that E >= V-1 is always satisfied.
    constexpr double EDGE_FACTOR = 2.0;

    std::mt19937 rng(42);

    // Linearly-spaced vertex counts
    std::vector<int> vertexCounts;
    vertexCounts.reserve(POINTS);
    for (int i = 0; i < POINTS; ++i)
    {
        double t = static_cast<double>(i) / (POINTS - 1);
        int v = static_cast<int>(std::round(V_MIN + t * (V_MAX - V_MIN)));
        vertexCounts.push_back(v);
    }
    vertexCounts.erase(std::unique(vertexCounts.begin(), vertexCounts.end()),
                       vertexCounts.end());

    const std::string outFile = "benchmark_results.csv";
    std::ofstream csv(outFile);
    if (!csv)
    {
        std::cerr << "Cannot open " << outFile << " for writing.\n";
        return 1;
    }

    csv << "vertices,edges,kruskal_ms,prim_ms\n";
    csv << std::fixed << std::setprecision(4);

    std::cout << "Benchmarking MST (sparse regime, E = " << EDGE_FACTOR << " * V)\n";
    std::cout << "V in [" << V_MIN << ", " << V_MAX << "], "
              << vertexCounts.size() << " samples, " << RUNS << " runs each\n";
    std::cout << std::string(66, '-') << "\n";
    std::cout << std::setw(8)  << "V"
              << std::setw(10) << "E"
              << std::setw(16) << "Kruskal (ms)"
              << std::setw(14) << "Prim (ms)" << "\n";
    std::cout << std::string(66, '-') << "\n";

    for (int v : vertexCounts)
    {
        int e = std::max(v - 1, static_cast<int>(std::round(EDGE_FACTOR * v)));

        Graph g = makeConnectedGraph(v, e, rng);

        double kruskal_ms = timeMedian(g, GraphAlgorithms::buildKruskalMST, RUNS);
        double prim_ms    = timeMedian(g, GraphAlgorithms::buildPrimMST,    RUNS);

        csv  << v << "," << e << "," << kruskal_ms << "," << prim_ms << "\n";

        std::cout << std::setw(8)  << v
                  << std::setw(10) << e
                  << std::setw(14) << std::fixed << std::setprecision(3) << kruskal_ms << " ms"
                  << std::setw(12) << prim_ms << " ms\n";
    }

    csv.close();
    std::cout << std::string(66, '-') << "\n";
    std::cout << "Results saved to " << outFile << "\n";
    return 0;
}
