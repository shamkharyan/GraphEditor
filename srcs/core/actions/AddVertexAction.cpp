#include "core/actions/AddVertexAction.h"

#include <cassert>

AddVertexAction::AddVertexAction(Graph& graph, float x, float y, const std::string& name) :
    m_graph(graph),
    m_x(x),
    m_y(y),
    m_name(name)
{
}

bool AddVertexAction::doAction()
{
    if (m_completed)
        return false;

    if (m_vertId == -1)
        m_vertId = m_graph.addVertex(m_x, m_y, m_name);
    else
        m_graph.addVertexWithID(m_vertId, m_x, m_y, m_name);

    m_completed = true;

    return true;
}

bool AddVertexAction::undoAction()
{
    if (!m_completed)
        return false;

    assert(m_vertId != -1);
    m_graph.removeVertex(m_vertId);
    m_completed = false;

    return true;
}

