#include "core/actions/AddEdgeAction.h"

#include <cassert>

AddEdgeAction::AddEdgeAction(Graph& graph, int startVertexid, int endVertexId, float weight, const std::string& name) :
    m_graph(graph),
    m_startVertexId(startVertexid),
    m_endVertexId(endVertexId),
    m_weight(weight),
    m_name(name)
{
}

bool AddEdgeAction::doAction()
{
    if (m_completed)
        return false;

    if (m_edgeId == -1)
        m_edgeId = m_graph.addEdge(m_startVertexId, m_endVertexId, m_weight, m_name);
    else
        m_graph.addEdgeWithID(m_edgeId, m_startVertexId, m_endVertexId, m_weight, m_name);

    m_completed = true;

    return true;
}

bool AddEdgeAction::undoAction()
{
    if (!m_completed)
        return false;

    assert(m_edgeId != -1);
    m_graph.removeVertex(m_edgeId);
    m_completed = false;

    return true;
}

