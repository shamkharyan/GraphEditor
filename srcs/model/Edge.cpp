#include "model/Edge.h"

Edge::Edge(int id, int startVertexId, int endVertexId, float weight, const std::string& name) :
    m_id(id),
    m_startVertexId(startVertexId),
    m_endVertexId(endVertexId),
    m_weight(weight),
    m_name(name)
{
}

bool Edge::operator==(const Edge& other) const
{
    return m_id == other.m_id;
}

bool Edge::operator!=(const Edge& other) const
{
    return !(*this == other);
}

int Edge::getID() const { return m_id; }
int Edge::getStartVertexID() const { return m_startVertexId; }
int Edge::getEndVertexID() const { return m_endVertexId; }
float Edge::getWeight() const { return m_weight; }
const std::string& Edge::getName() const { return m_name; }

void Edge::setStartVertexID(int id) { m_startVertexId = id; }
void Edge::setEndVertexID(int id) { m_endVertexId = id; }
void Edge::setWeight(float weight) { m_weight = weight; }
void Edge::setName(const std::string& name) { m_name = name; }
