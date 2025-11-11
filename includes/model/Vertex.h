#ifndef EUCLIDEAN_VERTEX_H
#define EUCLIDEAN_VERTEX_H

#include <string>

class EuclideanVertex
{
public:
    EuclideanVertex(float x = 0.f, float y = 0.f, const std::string& name = "");
    EuclideanVertex(const EuclideanVertex& other);
    EuclideanVertex(EuclideanVertex&& other) noexcept;
    EuclideanVertex& operator=(const EuclideanVertex& other);
    EuclideanVertex& operator=(EuclideanVertex&& other) noexcept;

    bool operator==(const EuclideanVertex& other) const;
    bool operator!=(const EuclideanVertex& other) const;
    float distance(const EuclideanVertex& other) const;

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
    static int m_nextId;

    int m_id;
    float m_x;
    float m_y;
    std::string m_name;
};

#endif // EUCLIDEAN_VERTEX_H
