#include "model/Graph.h"
#include "model/Vertex.h"
#include "model/Edge.h"

#include <stdexcept>
#include <unordered_set>

int Graph::addVertex(float x, float y, const std::string& name)
{
    int id = m_nextVertexId++;

    m_vertices.insert(std::make_pair(id, Vertex(id, x, y, name)));
    m_connectedEdgesIds[id] = {};

    return id;
}

int Graph::addVertexWithID(int id, float x, float y, const std::string& name)
{
    if (m_vertices.find(id) != m_vertices.end())
        throw std::runtime_error("Graph: Vertex with this id already exists!");

    m_vertices.insert(std::make_pair(id, Vertex(id, x, y, name)));
    m_connectedEdgesIds[id] = {};

    if (id >= m_nextVertexId)
        m_nextVertexId = id + 1;

    return id;
}

void Graph::removeVertex(int id)
{
    if (m_vertices.find(id) == m_vertices.end())
        return;

    std::unordered_set<int> edgesToRemove = m_connectedEdgesIds[id];

    for (int edgeId : edgesToRemove)
        removeEdge(edgeId);

    m_vertices.erase(id);
    m_connectedEdgesIds.erase(id);
}

int Graph::addEdge(int startVertexId, int endVertexId, float weight, const std::string& name)
{
    auto startIt = m_vertices.find(startVertexId);
    auto endIt = m_vertices.find(endVertexId);
    if (startIt == m_vertices.end() || endIt == m_vertices.end())
        throw std::runtime_error("Graph: Invalid vertex id");

    int id = m_nextEdgeId++;

    m_edges.insert(std::make_pair(id, Edge(id, startIt->first, endIt->first, weight, name)));

    m_connectedEdgesIds[startIt->first].insert(id);
    m_connectedEdgesIds[endIt->first].insert(id);

    return id;
}

int Graph::addEdgeWithID(int id, int startVertexId, int endVertexId, float weight, const std::string& name)
{
    if (m_edges.find(id) != m_edges.end())
        throw std::runtime_error("Graph: Edge with this id already exists!");

    auto startIt = m_vertices.find(startVertexId);
    auto endIt = m_vertices.find(endVertexId);
    if (startIt == m_vertices.end() || endIt == m_vertices.end())
        throw std::runtime_error("Graph: Invalid vertex id");

    m_edges.insert(std::make_pair(id, Edge(id, startIt->first, endIt->first, weight, name)));

    m_connectedEdgesIds[startIt->first].insert(id);
    m_connectedEdgesIds[endIt->first].insert(id);

    if (id >= m_nextEdgeId)
        m_nextEdgeId = id + 1;

    return id;
}

void Graph::removeEdge(int id)
{
    auto it = m_edges.find(id);
    if (it == m_edges.end())
        return;

    int startVertexId = it->second.getStartVertexID();
    int endVertexId = it->second.getEndVertexID();
    m_edges.erase(it);

    m_connectedEdgesIds[startVertexId].erase(id);
    m_connectedEdgesIds[endVertexId].erase(id);
}

const Graph::VertexMap& Graph::getVertices() const { return m_vertices; }
const Graph::EdgeMap& Graph::getEdges() const { return m_edges; }
const Graph::ConnectedEdgesMap& Graph::getConnectedEdgesIds() const { return m_connectedEdgesIds; }
