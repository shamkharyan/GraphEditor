#include "model/EuclideanVertex.h"

#include <cmath>

int EuclideanVertex::m_nextId = 0;

EuclideanVertex::EuclideanVertex(float x, float y, const std::string& name) :
    m_x(x), m_y(y), m_name(name)
{
    m_id = m_nextId++;
}

EuclideanVertex::EuclideanVertex(const EuclideanVertex& other) :
    m_x(other.m_x), m_y(other.m_y), m_name(other.m_name)
{
    m_id = m_nextId++;
}

EuclideanVertex::EuclideanVertex(EuclideanVertex&& other) noexcept :
    m_x(other.m_x), m_y(other.m_y), m_name(std::move(other.m_name))
{
    m_id = m_nextId++;
}

EuclideanVertex& EuclideanVertex::operator=(const EuclideanVertex& other)
{
    if (this == &other)
        return *this;

    m_x = other.m_x;
    m_y = other.m_y;
    m_name = other.m_name;

    return *this;
}

EuclideanVertex& EuclideanVertex::operator=(EuclideanVertex&& other) noexcept
{
    if (this == &other)
        return *this;

    m_x = other.m_x;
    m_y = other.m_y;
    m_name = std::move(other.m_name);

    return *this;
}

bool EuclideanVertex::operator==(const EuclideanVertex& other) const
{
    return m_id == other.m_id;
}

bool EuclideanVertex::operator!=(const EuclideanVertex& other) const
{
    return !(*this == other);
}

float EuclideanVertex::distance(const EuclideanVertex& other) const
{
    int dx = m_x - other.m_x;
    int dy = m_y - other.m_y;

    return std::sqrt(dx * dx + dy * dy);
}

const std::string& EuclideanVertex::getName() const { return m_name; }
int EuclideanVertex::getID() const { return m_id; }
float EuclideanVertex::getX() const { return m_x; }
float EuclideanVertex::getY() const { return m_y; }

void EuclideanVertex::setName(const std::string& name) { m_name = name; }
void EuclideanVertex::setX(float x) { m_x = x; }
void EuclideanVertex::setY(float y) { m_y = y; }
