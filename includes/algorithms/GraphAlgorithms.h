#ifndef GRAPH_ALGORITHMS_H
#define GRAPH_ALGORITHMS_H

#include "../model/Graph.h"
#include "MSTModelSelector.h"

class GraphAlgorithms
{
public:
    static Graph buildKruskalMST(const Graph& graph);
    static Graph buildPrimMST(const Graph& graph);

    // Picks Kruskal for sparse/large graphs, Prim for dense/small graphs.
    // Threshold: if |E| < |V| * log2(|V|) use Kruskal, else Prim.
    static Graph buildAutoMST(const Graph& graph);
};

#endif // GRAPH_ALGORITHMS_H
