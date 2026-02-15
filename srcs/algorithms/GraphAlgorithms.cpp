#include "algorithms/GraphAlgorithms.h"
#include "model/DisjointSet.h"
#include <vector>
#include <algorithm>
#include <queue>
#include <limits>
#include <unordered_map>
#include <numeric>
#include <functional>
#include <cmath>
#include <stdexcept>

// ============================================================================
// Kruskal's MST  —  O(E log E)
//
// Improvements over the original:
//   1. Sort a compact (weight, u, v) struct instead of copying full Edge objects.
//      The sort touches ~12 bytes per element vs ~48; better cache utilisation.
//   2. Union-Find uses a flat int[] array indexed by a dense 0-based remapping
//      of vertex IDs instead of DisjointSet<int>'s unordered_map per lookup.
//      Every find/union is now pure array indexing — no hashing, no branching.
//   3. Early termination: stop as soon as V-1 edges have been added.
//      For sparse graphs this avoids scanning the long tail of rejected edges.
// ============================================================================

Graph GraphAlgorithms::buildKruskalMST(const Graph& graph)
{
    const auto& vertexMap = graph.getVertices();
    const auto& edgeMap   = graph.getEdges();

    const int V = static_cast<int>(vertexMap.size());
    const int E = static_cast<int>(edgeMap.size());

    if (V == 0) return Graph{};

    // ── Dense vertex ID remapping ────────────────────────────────────────────
    // Vertex IDs may not be 0-based or contiguous. Map them to [0, V) once
    // so that all subsequent operations index into flat arrays.
    std::unordered_map<int, int> toIdx;
    toIdx.reserve(V);
    std::vector<int> toId(V);
    {
        int i = 0;
        for (const auto& [id, _] : vertexMap)
        {
            toIdx[id] = i;
            toId[i]   = id;
            ++i;
        }
    }

    // ── Compact edge list ────────────────────────────────────────────────────
    struct KEdge { float w; int u, v, origId; };
    std::vector<KEdge> edges;
    edges.reserve(E);
    for (const auto& [id, e] : edgeMap)
        edges.push_back({ e.getWeight(),
                          toIdx[e.getStartVertexID()],
                          toIdx[e.getEndVertexID()],
                          id });

    std::sort(edges.begin(), edges.end(),
              [](const KEdge& a, const KEdge& b){ return a.w < b.w; });

    // ── Flat union-find: parent[i] >= 0 → parent; < 0 → -size (root) ────────
    std::vector<int> parent(V, -1);

    // Iterative path-halving find — no recursion, no std::function overhead
    auto find = [&](int x) -> int {
        while (parent[x] >= 0)
        {
            if (parent[parent[x]] >= 0)
                parent[x] = parent[parent[x]];  // path halving
            x = parent[x];
        }
        return x;
    };

    auto unite = [&](int x, int y) -> bool {
        x = find(x); y = find(y);
        if (x == y) return false;
        if (parent[x] > parent[y]) std::swap(x, y);   // union by size
        parent[x] += parent[y];
        parent[y]  = x;
        return true;
    };

    // ── Build MST ────────────────────────────────────────────────────────────
    // Collect chosen edges first, build Graph output after the loop to avoid
    // paying Graph's internal bookkeeping inside the hot path.
    struct ChosenEdge { float w; int u, v; };
    std::vector<ChosenEdge> chosen;
    chosen.reserve(V - 1);

    for (const KEdge& e : edges)
    {
        if (unite(e.u, e.v))
        {
            chosen.push_back({ e.w, toId[e.u], toId[e.v] });
            if (static_cast<int>(chosen.size()) == V - 1)
                break;   // MST complete — skip remaining edges
        }
    }

    if (static_cast<int>(chosen.size()) < V - 1)
        throw std::runtime_error(
            "Graph is disconnected: MST does not exist for the entire graph.");

    // ── Assemble output graph ────────────────────────────────────────────────
    Graph mst;
    for (const auto& [id, vertex] : vertexMap)
        mst.addVertexWithID(id, vertex.getX(), vertex.getY(), vertex.getName());
    for (const ChosenEdge& e : chosen)
        mst.addEdge(e.u, e.v, e.w);

    return mst;
}

// ============================================================================
// Prim's MST  —  O(E log V)  (binary min-heap + compact adjacency list)
//
// Improvements over the original:
//   1. Replaces the O(V) linear scan for the minimum-cost vertex with a
//      std::priority_queue (binary min-heap) — total complexity drops from
//      O(V²) to O(E log V).  This is the single most impactful change.
//   2. Pre-builds a compact adjacency list as vector<vector<PAdj>> before
//      the main loop.  Each PAdj stores (weight, neighbor_idx, origEdgeId)
//      as plain integers. The inner relaxation loop therefore does pure
//      array reads — zero hash-map lookups per edge visit.
//   3. "inMST" is a flat bool[] — single array read for membership test.
//   4. "key" is a flat float[] — all vertex state in contiguous memory.
//   5. Lazy-deletion heap: stale entries skipped by inMST[] check,
//      avoiding the expensive decrease-key operation.
// ============================================================================

Graph GraphAlgorithms::buildPrimMST(const Graph& graph)
{
    const auto& vertexMap      = graph.getVertices();
    const auto& edgeMap        = graph.getEdges();
    const auto& connectedEdges = graph.getConnectedEdgesIds();

    const int V = static_cast<int>(vertexMap.size());
    if (V == 0) return Graph{};

    // ── Dense vertex ID remapping (done once, before the hot path) ───────────
    std::unordered_map<int, int> toIdx;
    toIdx.reserve(V);
    std::vector<int> toId(V);
    {
        int i = 0;
        for (const auto& [id, _] : vertexMap)
        {
            toIdx[id] = i;
            toId[i]   = id;
            ++i;
        }
    }

    // ── Compact adjacency list — built once, accessed with zero hashing ──────
    // adj[u] holds all edges from compact-vertex u as (weight, v_idx, orig_edgeId)
    struct PAdj { float w; int v, edgeId; };
    std::vector<std::vector<PAdj>> adj(V);
    // Reserve average degree to avoid repeated reallocation
    {
        const int E = static_cast<int>(edgeMap.size());
        const int avgDeg = (V > 0) ? std::max(1, (2 * E) / V) : 1;
        for (auto& a : adj) a.reserve(avgDeg);
    }
    for (const auto& [id, edge] : edgeMap)
    {
        int u = toIdx[edge.getStartVertexID()];
        int v = toIdx[edge.getEndVertexID()];
        float w = edge.getWeight();
        adj[u].push_back({w, v, id});
        adj[v].push_back({w, u, id});
    }

    // ── Flat per-vertex state ────────────────────────────────────────────────
    constexpr float INF = std::numeric_limits<float>::max();
    std::vector<float> key(V, INF);
    std::vector<int>   parentEdge(V, -1);
    std::vector<bool>  inMST(V, false);

    // ── Min-heap: (key, vertex_index) ───────────────────────────────────────
    using HeapEntry = std::pair<float, int>;
    std::priority_queue<HeapEntry,
                        std::vector<HeapEntry>,
                        std::greater<HeapEntry>> pq;

    key[0] = 0.0f;
    pq.push({0.0f, 0});

    int edgesAdded = 0;

    while (!pq.empty() && edgesAdded < V - 1)
    {
        auto [cost, u] = pq.top();
        pq.pop();

        if (inMST[u]) continue;   // lazy-deletion: stale entry
        inMST[u] = true;
        if (parentEdge[u] != -1) ++edgesAdded;

        // Inner loop: pure array access, no hash lookups
        for (const PAdj& nb : adj[u])
        {
            if (!inMST[nb.v] && nb.w < key[nb.v])
            {
                key[nb.v]        = nb.w;
                parentEdge[nb.v] = nb.edgeId;
                pq.push({nb.w, nb.v});
            }
        }
    }

    // ── Connectivity check ───────────────────────────────────────────────────
    for (int i = 0; i < V; ++i)
        if (!inMST[i])
            throw std::runtime_error(
                "Graph is disconnected: MST does not exist for the entire graph.");

    // ── Assemble output graph ────────────────────────────────────────────────
    Graph mst;
    for (const auto& [id, vertex] : vertexMap)
        mst.addVertexWithID(id, vertex.getX(), vertex.getY(), vertex.getName());

    for (int i = 1; i < V; ++i)
    {
        const auto& edge = edgeMap.at(parentEdge[i]);
        mst.addEdge(edge.getStartVertexID(),
                    edge.getEndVertexID(),
                    edge.getWeight(),
                    edge.getName());
    }

    return mst;
}

// ============================================================================
// Auto-select: Kruskal for sparse, Prim for dense
// Both are now O(E log E) / O(E log V) so the crossover point shifts
// compared to the old O(V²) Prim — use graph density as the heuristic.
// ============================================================================

Graph GraphAlgorithms::buildAutoMST(const Graph& graph)
{
    const std::size_t V = graph.getVertices().size();
    const std::size_t E = graph.getEdges().size();

    // Dense when E > V * log2(V).
    // Below that Kruskal's smaller constant tends to win;
    // above it Prim's O(E log V) vs Kruskal's O(E log E) cost differs only
    // by the log factor, but Prim's tighter inner loop wins in practice.
    const auto logV = static_cast<std::size_t>(
        V > 1 ? std::log2(static_cast<double>(V)) : 1.0);

    if (E > V * logV)
        return buildPrimMST(graph);
    else
        return buildKruskalMST(graph);
}
