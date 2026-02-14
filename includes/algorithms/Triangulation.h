#ifndef TRIANGULATION_H
#define TRIANGULATION_H

#include "model/Graph.h"
#include <vector>

struct Triangle
{
    int v1, v2, v3;  // Vertex IDs (-1/-2/-3 = super-triangle vertices)

    Triangle(int a, int b, int c) : v1(a), v2(b), v3(c) {}

    bool hasSuperVertex() const { return v1 < 0 || v2 < 0 || v3 < 0; }
    bool containsVertex(int id) const { return v1 == id || v2 == id || v3 == id; }

    struct Edge
    {
        int a, b;
        Edge(int x, int y) : a(x < y ? x : y), b(x < y ? y : x) {}
        bool operator==(const Edge& o) const { return a == o.a && b == o.b; }
    };
};

class DelaunayTriangulation
{
public:
    // Returns triangles of the Delaunay triangulation (super-triangle removed).
    static std::vector<Triangle> triangulate(const Graph& graph);

    // Returns a new graph with all triangulation edges added (weight = euclidean distance).
    static Graph buildTriangulatedGraph(const Graph& graph);

private:
    struct Point { int id; float x, y; };

    struct Circumcircle { float cx, cy, r2; };

    static Circumcircle computeCircumcircle(const Point& a, const Point& b, const Point& c);
    static bool         inCircumcircle(const Circumcircle& cc, float px, float py);
    static void         addSuperTriangle(std::vector<Point>& pts, float margin);
};

#endif // TRIANGULATION_H
