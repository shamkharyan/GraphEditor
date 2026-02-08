#ifndef EDGE_H
#define EDGE_H

#include <string>

class Edge
{
public:
    Edge() = default;
    Edge(int id, int startVertexId, int endVertexId, float weight = 1.0f, const std::string& name = "");

    bool operator==(const Edge& other) const;
    bool operator!=(const Edge& other) const;

    int getID() const;
    int getStartVertexID() const;
    int getEndVertexID() const;
    float getWeight() const;
    const std::string& getName() const;

    void setStartVertexID(int id);
    void setEndVertexID(int id);
    void setWeight(float weight);
    void setName(const std::string& name);

private:
    int m_id = -1;
    int m_startVertexId = -1;
    int m_endVertexId = -1;
    float m_weight = 1.0f;
    std::string m_name;
};

#endif // EDGE_H
