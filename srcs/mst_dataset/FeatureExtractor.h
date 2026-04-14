#ifndef MST_DATASET_FEATURE_EXTRACTOR_H
#define MST_DATASET_FEATURE_EXTRACTOR_H

#include "model/Graph.h"

struct GraphFeatures
{
    int n = 0;
    int m = 0;
    double density = 0.0;
    double avgDegree = 0.0;
    double logN = 0.0;
    double logM = 0.0;
    double meanWeight = 0.0;
    double stdWeight = 0.0;
    double minWeight = 0.0;
    double maxWeight = 0.0;
    double skewWeight = 0.0;
    int connectedComponents = 0;
};

class FeatureExtractor
{
public:
    static GraphFeatures extract(const Graph& graph);
};

#endif // MST_DATASET_FEATURE_EXTRACTOR_H
