#ifndef EUCLIDEAN_VERTEX_H
#define EUCLIDEAN_VERTEX_H

#include <string>

class EuclideanVertex
{
public:
    EuclideanVertex(int x, int y, const std::string& name = "");

    bool operator==(const EuclideanVertex& other) const;
    bool operator!=(const EuclideanVertex& other) const;
    float distance(const EuclideanVertex& other) const;

    // Getters
    int getX() const;
    int getY() const;
    const std::string& getName() const;

    // Setters
    void setX(int x);
    void setY(int y);
    void setName(const std::string& name);

private:
    int m_x;
    int m_y;
    std::string m_name;
};

#endif // EUCLIDEAN_VERTEX_H
