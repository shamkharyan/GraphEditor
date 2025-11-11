#include "model/Vertex.h"

#include <cmath>

int Vertex::m_nextId = 0;

Vertex::Vertex(float x, float y, const std::string& name) :
    m_x(x), m_y(y), m_name(name)
{
    m_id = m_nextId++;
}

Vertex::Vertex(const Vertex& other) :
    m_x(other.m_x), m_y(other.m_y), m_name(other.m_name)
{
    m_id = m_nextId++;
}

Vertex::Vertex(Vertex&& other) noexcept :
    m_x(other.m_x), m_y(other.m_y), m_name(std::move(other.m_name))
{
    m_id = m_nextId++;
}

Vertex& Vertex::operator=(const Vertex& other)
{
    if (this == &other)
        return *this;

    m_x = other.m_x;
    m_y = other.m_y;
    m_name = other.m_name;

    return *this;
}

Vertex& Vertex::operator=(Vertex&& other) noexcept
{
    if (this == &other)
        return *this;

    m_x = other.m_x;
    m_y = other.m_y;
    m_name = std::move(other.m_name);

    return *this;
}

bool Vertex::operator==(const Vertex& other) const
{
    return m_id == other.m_id;
}

bool Vertex::operator!=(const Vertex& other) const
{
    return !(*this == other);
}

float Vertex::distance(const Vertex& other) const
{
    int dx = m_x - other.m_x;
    int dy = m_y - other.m_y;

    return std::sqrt(dx * dx + dy * dy);
}

const std::string& Vertex::getName() const { return m_name; }
int Vertex::getID() const { return m_id; }
float Vertex::getX() const { return m_x; }
float Vertex::getY() const { return m_y; }

void Vertex::setName(const std::string& name) { m_name = name; }
void Vertex::setX(float x) { m_x = x; }
void Vertex::setY(float y) { m_y = y; }
