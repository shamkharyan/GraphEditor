#ifndef ADD_VERTEX_ACTION_H
#define ADD_VERTEX_ACTION_H

#include "core/actions/AAction.h"
#include "model/Graph.h"

class AddVertexAction : public AAction
{
public:
    AddVertexAction(Graph& graph, float x, float y, const std::string& name);

    bool doAction() override;
    bool undoAction() override;
private:
    Graph& m_graph;

    float m_x;
    float m_y;
    const std::string& m_name;

    int m_vertId = -1;
};

#endif // ADD_VERTEX_ACTION_H
