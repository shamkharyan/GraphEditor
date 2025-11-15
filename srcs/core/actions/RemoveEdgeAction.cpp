#include "core/actions/RemoveEdgeAction.h"

#include <cassert>

RemoveEdgeAction::RemoveEdgeAction(Graph& graph, int id) :
    m_graph(graph),
    m_id(id)
{
}

bool RemoveEdgeAction::doAction()
{
    if (m_completed)
        return false;

    const Edge& edge = m_graph.getEdge(m_id);

    m_oldName = edge.getName();
    m_oldStartVertex = edge.getStartVertexID();
    m_oldEndVertex = edge.getEndVertexID();
    m_oldWeight = edge.getWeight();

    m_graph.removeEdge(m_id);
    m_completed = true;

    return true;
}

bool RemoveEdgeAction::undoAction()
{
    if (!m_completed)
        return false;

    m_graph.addEdgeWithID(m_id, m_oldStartVertex, m_oldEndVertex, m_oldWeight, m_oldName);
    m_completed = false;

    return true;
}

