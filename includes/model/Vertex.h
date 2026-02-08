#ifndef VERTEX_H
#define VERTEX_H

#include <string>

class Vertex
{
public:
    Vertex() = default;
    Vertex(int id, float x, float y, const std::string& name = "");

    bool operator==(const Vertex& other) const;
    bool operator!=(const Vertex& other) const;

    float distance(const Vertex& other) const;

    int getID() const;
    float getX() const;
    float getY() const;
    const std::string& getName() const;

    void setX(float x);
    void setY(float y);
    void setName(const std::string& name);

private:
    int m_id = -1;
    float m_x = 0.0f;
    float m_y = 0.0f;
    std::string m_name;
};

#endif // VERTEX_H
