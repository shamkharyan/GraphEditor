#ifndef GRAPH_ALGORITHMS_H
#define GRAPH_ALGORITHMS_H

#include "model/Graph.h"

class GraphAlgorithms
{
public:
    static Graph buildKruskalMST(const Graph& graph);
    static Graph buildPrimMST(const Graph& graph);
};

#endif // GRAPH_ALGORITHMS_H
