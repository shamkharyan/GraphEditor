#ifndef VERTEX_H
#define VERTEX_H

#include <string>

class Vertex
{
public:
    Vertex(int id, float x = 0.f, float y = 0.f, const std::string& name = "");

    bool operator==(const Vertex& other) const;
    bool operator!=(const Vertex& other) const;
    float distance(const Vertex& other) const;

    // Getters
    int getID() const;
    float getX() const;
    float getY() const;
    const std::string& getName() const;

    // Setters
    void setX(float x);
    void setY(float y);
    void setName(const std::string& name);

private:
    int m_id;
    float m_x;
    float m_y;
    std::string m_name;
};

#endif // VERTEX_H
