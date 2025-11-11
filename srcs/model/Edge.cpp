#include "model/Edge.h"

int Edge::m_nextId = 0;

Edge::Edge(int startVertexId, int endVertexId, float weight, const std::string& name = "") :
    m_startVertexId(startVertexId), m_endVertexId(endVertexId), m_weight(weight), m_name(name)
{
    m_id = m_nextId++;
}

Edge::Edge(const Edge& other) : m_startVertexId(other.m_startVertexId), m_endVertexId(other.m_endVertexId),
    m_weight(other.m_weight), m_name(other.m_name)
{
    m_id = m_nextId++;
}

Edge::Edge(Edge&& other) noexcept : m_startVertexId(other.m_startVertexId), m_endVertexId(other.m_endVertexId),
    m_weight(other.m_weight), m_name(std::move(other.m_name))
{
    m_id = m_nextId++;
}

Edge& Edge::operator=(const Edge& other)
{
    if (this == &other)
        return *this;

    m_startVertexId = other.m_startVertexId;
    m_endVertexId = other.m_endVertexId;
    m_weight = other.m_weight;
    m_name = other.m_name;

    return *this;
}

Edge& Edge::operator=(Edge&& other) noexcept
{
    if (this == &other)
        return *this;

    m_startVertexId = other.m_startVertexId;
    m_endVertexId = other.m_endVertexId;
    m_weight = other.m_weight;
    m_name = std::move(other.m_name);

    return *this;
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
const Edge::std::string& getName() const { return m_name; }

void Edge::setStartVertexID(int id) { m_startVertexId = id; }
void Edge::setEndVertexID(int id) { m_endVertexId = id; }
void Edge::setWeight(float weight) { m_weight = weight; }
void Edge::setName(const std::string& name) { m_name = name; }
