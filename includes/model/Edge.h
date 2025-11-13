#ifndef EDGE_H
#define EDGE_H

#include <string>

class Edge
{
public:
    Edge(int id, int startVertexId, int endVertexId, float weight, const std::string& name = "");

    bool operator==(const Edge& other) const;
    bool operator!=(const Edge& other) const;

    // Getters
    int getID() const;
    int getStartVertexID() const;
    int getEndVertexID() const;
    float getWeight() const;
    const std::string& getName() const;

    // Setters
    void setStartVertexID(int id);
    void setEndVertexID(int id);
    void setWeight(float weight);
    void setName(const std::string& name);

private:
    int m_id;
    int m_startVertexId;
    int m_endVertexId;
    float m_weight;
    std::string m_name;
};

#endif // EDGE_H
