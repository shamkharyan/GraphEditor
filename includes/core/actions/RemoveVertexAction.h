#ifndef REMOVE_VERTEX_ACTION_H
#define REMOVE_VERTEX_ACTION_H

#include "core/actions/AAction.h"
#include "model/Graph.h"

class RemoveVertexAction : public AAction
{
public:
    RemoveVertexAction(Graph& graph, int id);

    bool doAction() override;
    bool undoAction() override;
private:
    Graph& m_graph;

    int m_id;

    float m_oldX;
    float m_oldY;
    std::string m_oldName;
};

#endif // REMOVE_VERTEX_ACTION_H
