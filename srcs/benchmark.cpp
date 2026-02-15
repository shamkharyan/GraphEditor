// Standalone MST benchmark — Kruskal vs Prim
// 1000 vertices, sweep edge count from sparse to dense
// Output: benchmark_results.csv  (edges, kruskal_ms, prim_ms)
//
// Build (from graph_editor root):
//   g++ -O2 -std=c++17 -I. benchmark.cpp model/Graph.cpp model/Vertex.cpp model/Edge.cpp \
//       algorithms/GraphAlgorithms.cpp -o benchmark

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
// Builds a connected random graph with exactly V vertices and E edges.
// Strategy:
//   1. Build a random spanning tree (V-1 edges) — guarantees connectivity.
//   2. Add random extra edges until we reach E total.
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

    // Step 1: random spanning tree via random-insertion shuffle
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

    // Step 2: fill remaining edges randomly
    std::uniform_int_distribution<int> vD(0, V - 1);
    int maxAttempts = (E - (V - 1)) * 20 + 1000;
    int attempts = 0;
    while ((int)g.getEdges().size() < E && attempts < maxAttempts)
    {
        ++attempts;
        int a = vD(rng), b = vD(rng);
        if (a == b) continue;
        if (a > b) std::swap(a, b);
        if (used.insert({a, b}).second)
            g.addEdge(a, b, wD(rng));
    }

    return g;
}

// ── Timer ───────────────────────────────────────────────────────────────────
// Runs algo RUNS times and returns the median elapsed time in milliseconds.
template<typename Algo>
double timeMedian(const Graph& g, Algo algo, int runs = 5)
{
    std::vector<double> times;
    times.reserve(runs);
    for (int i = 0; i < runs; ++i)
    {
        auto t0 = Clock::now();
        try { algo(g); } catch (...) {}   // disconnected check shouldn't fire
        auto t1 = Clock::now();
        times.push_back(Ms(t1 - t0).count());
    }
    std::sort(times.begin(), times.end());
    return times[runs / 2];
}

// ── Main ────────────────────────────────────────────────────────────────────
int main()
{
    constexpr long long V       = 1000;
    constexpr long long RUNS    = 5;     // runs per data point (median taken)
    constexpr long long POINTS  = 40;    // number of edge-count samples

    const long long E_min = V - 1;               // minimum: spanning tree
    const long long E_max = V * (V - 1) / 2;    // maximum: complete graph

    std::mt19937 rng(42);   // fixed seed → reproducible

    // Logarithmically-spaced edge counts give good coverage of sparse→dense
    std::vector<long long> edgeCounts;
    for (int i = 0; i < POINTS; ++i)
    {
        double t  = static_cast<double>(i) / (POINTS - 1);
        // log-space between E_min and E_max
        double lv = std::log2(E_min) + t * (std::log2(E_max) - std::log2(E_min));
        long long e = static_cast<long long>(std::round(std::pow(2.0, lv)));
        e = std::max(E_min, std::min(e, E_max));
        edgeCounts.push_back(e);
    }
    // remove duplicates while preserving order
    edgeCounts.erase(std::unique(edgeCounts.begin(), edgeCounts.end()),
                     edgeCounts.end());

    const std::string outFile = "benchmark_results.csv";
    std::ofstream csv(outFile);
    if (!csv)
    {
        std::cerr << "Cannot open " << outFile << " for writing.\n";
        return 1;
    }

    csv << "edges,kruskal_ms,prim_ms\n";
    csv << std::fixed << std::setprecision(4);

    std::cout << "Benchmarking MST algorithms: V=" << V
              << ", " << edgeCounts.size() << " edge-count samples, "
              << RUNS << " runs each\n";
    std::cout << std::string(60, '-') << "\n";
    std::cout << std::setw(10) << "Edges"
              << std::setw(16) << "Kruskal (ms)"
              << std::setw(14) << "Prim (ms)" << "\n";
    std::cout << std::string(60, '-') << "\n";

    for (int e : edgeCounts)
    {
        Graph g = makeConnectedGraph(V, e, rng);

        double kruskal_ms = timeMedian(g, GraphAlgorithms::buildKruskalMST, RUNS);
        double prim_ms    = timeMedian(g, GraphAlgorithms::buildPrimMST,    RUNS);

        csv  << e << "," << kruskal_ms << "," << prim_ms << "\n";

        std::cout << std::setw(10) << e
                  << std::setw(14) << std::fixed << std::setprecision(3) << kruskal_ms << " ms"
                  << std::setw(12) << prim_ms << " ms\n";
    }

    csv.close();
    std::cout << std::string(60, '-') << "\n";
    std::cout << "Results saved to " << outFile << "\n";
    return 0;
}
