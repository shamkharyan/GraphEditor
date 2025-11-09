#include "model/EuclideanVertex.h"

#include <cmath>

EuclideanVertex::EuclideanVertex(int x, int y, const std::string& name) : m_x(x), m_y(y), m_name(name) {}

bool EuclideanVertex::operator==(const EuclideanVertex& other) const
{
    return m_x == other.m_x && m_y == other.m_y;
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
int EuclideanVertex::getX() const { return m_x; }
int EuclideanVertex::getY() const { return m_y; }

void EuclideanVertex::setName(const std::string& name) { m_name = name; }
void EuclideanVertex::setX(int x) { m_x = x; }
void EuclideanVertex::setY(int y) { m_y = y; }
