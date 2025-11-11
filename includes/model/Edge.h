#ifndef EDGE_H
#define EDGE_H

#include <string>

class Edge
{
public:
    Edge(int startVertexId, int endVertexId, float weight, const std::string& name = "");

    Edge(const Edge& other);
    Edge(Edge&& other) noexcept;
    Edge& operator=(const Edge& other);
    Edge& operator=(Edge&& other) noexcept;

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
    static int m_nextId;

    int m_id;
    int m_startVertexId;
    int m_endVertexId;
    float m_weight;
    std::string m_name;
};

#endif // EDGE_H
