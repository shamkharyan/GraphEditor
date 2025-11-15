#ifndef ADD_EDGE_ACTION_H
#define ADD_EDGE_ACTION_H

#include "core/actions/AAction.h"
#include "model/Graph.h"

class AddEdgeAction : public AAction
{
public:
    AddEdgeAction(Graph& graph, int startVertexid, int endVertexId, float weight, const std::string& name);

    bool doAction() override;
    bool undoAction() override;
private:
    Graph& m_graph;

    int m_startVertexId;
    int m_endVertexId;
    float m_weight;
    const std::string& m_name;

    int m_edgeId = -1;
};

#endif // ADD_EDGE_ACTION_H
