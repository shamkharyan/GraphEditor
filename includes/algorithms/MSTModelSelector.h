#ifndef MST_MODEL_SELECTOR_H
#define MST_MODEL_SELECTOR_H

#include "model/Graph.h"

#include <string>

class MSTModelSelector
{
public:
    enum class Algorithm
    {
        Kruskal,
        Prim
    };

    static Algorithm predict(const Graph& graph);
    static bool isModelAvailable();
    static std::string modelStatus();

private:
    static Algorithm predictWithHeuristic(const Graph& graph);
};

#endif // MST_MODEL_SELECTOR_H
