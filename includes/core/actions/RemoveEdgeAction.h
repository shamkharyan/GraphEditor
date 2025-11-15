#ifndef REMOVE_EDGE_ACTION_H
#define REMOVE_EDGE_ACTION_H

#include "core/actions/AAction.h"
#include "model/Graph.h"

class RemoveEdgeAction : public AAction
{
public:
    RemoveEdgeAction(Graph& graph, int id);

    bool doAction() override;
    bool undoAction() override;
private:
    Graph& m_graph;

    int m_id;

    int m_oldStartVertex = -1;
    int m_oldEndVertex = -1;
    float m_oldWeight;
    std::string m_oldName;
};

#endif // REMOVE_EDGE_ACTION_H
