#include "core/actions/RemoveVertexAction.h"

#include <cassert>

RemoveVertexAction::RemoveVertexAction(Graph& graph, int id) :
    m_graph(graph),
    m_id(id)
{
}

bool RemoveVertexAction::doAction()
{
    if (m_completed)
        return false;

    const Vertex& vertex = m_graph.getVertex(m_id);

    m_oldName = vertex.getName();
    m_oldX = vertex.getX();
    m_oldY = vertex.getY();

    m_graph.removeVertex(m_id);
    m_completed = true;

    return true;
}

bool RemoveVertexAction::undoAction()
{
    if (!m_completed)
        return false;

    m_graph.addVertexWithID(m_id, m_oldX, m_oldY, m_oldName);
    m_completed = false;

    return true;
}

