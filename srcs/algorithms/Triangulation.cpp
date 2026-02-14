#include "algorithms/Triangulation.h".h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <unordered_map>
#include <set>

// ---------------------------------------------------------------------------
// Circumcircle helpers
// ---------------------------------------------------------------------------

DelaunayTriangulation::Circumcircle
DelaunayTriangulation::computeCircumcircle(const Point& a, const Point& b, const Point& c)
{
    // Using the standard perpendicular-bisector formula.
    // For numerical stability we shift coordinates so that `a` is at origin.
    const float ax = b.x - a.x;
    const float ay = b.y - a.y;
    const float bx = c.x - a.x;
    const float by = c.y - a.y;

    const float D = 2.0f * (ax * by - ay * bx);

    Circumcircle cc;

    if (std::abs(D) < 1e-10f)
    {
        // Degenerate (collinear) triangle – put circumcentre far away
        cc.cx = std::numeric_limits<float>::max() / 2.0f;
        cc.cy = std::numeric_limits<float>::max() / 2.0f;
        cc.r2 = std::numeric_limits<float>::max();
        return cc;
    }

    const float ux = (by * (ax * ax + ay * ay) - ay * (bx * bx + by * by)) / D;
    const float uy = (ax * (bx * bx + by * by) - bx * (ax * ax + ay * ay)) / D;

    cc.cx = a.x + ux;
    cc.cy = a.y + uy;
    cc.r2 = ux * ux + uy * uy;

    return cc;
}

bool DelaunayTriangulation::inCircumcircle(const Circumcircle& cc, float px, float py)
{
    const float dx = px - cc.cx;
    const float dy = py - cc.cy;
    return (dx * dx + dy * dy) < cc.r2 - 1e-10f;
}

// ---------------------------------------------------------------------------
// Super-triangle: three artificial vertices that encompass all real points.
// Their ids are -1, -2, -3 so they are distinct from any valid vertex id.
// ---------------------------------------------------------------------------

void DelaunayTriangulation::addSuperTriangle(std::vector<Point>& pts, float margin)
{
    if (pts.empty()) return;

    float minX = pts[0].x, maxX = pts[0].x;
    float minY = pts[0].y, maxY = pts[0].y;

    for (const auto& p : pts)
    {
        if (p.x < minX) minX = p.x;
        if (p.x > maxX) maxX = p.x;
        if (p.y < minY) minY = p.y;
        if (p.y > maxY) maxY = p.y;
    }

    const float dx   = (maxX - minX) + margin;
    const float dy   = (maxY - minY) + margin;
    const float midX = (minX + maxX) * 0.5f;
    const float midY = (minY + maxY) * 0.5f;

    // A large equilateral-ish triangle
    pts.push_back({ -1, midX,           midY - 20.0f * dy });
    pts.push_back({ -2, midX - 20.0f * dx, midY + 20.0f * dy });
    pts.push_back({ -3, midX + 20.0f * dx, midY + 20.0f * dy });
}

// ---------------------------------------------------------------------------
// Bowyer-Watson algorithm
// ---------------------------------------------------------------------------

std::vector<Triangle> DelaunayTriangulation::triangulate(const Graph& graph)
{
    const auto& vertices = graph.getVertices();
    if (vertices.size() < 3)
        return {};

    // Build a flat list of points (we will append 3 super-triangle points)
    std::vector<Point> pts;
    pts.reserve(vertices.size() + 3);

    for (const auto& [id, v] : vertices)
        pts.push_back({ id, v.getX(), v.getY() });

    // Add super-triangle vertices at indices n, n+1, n+2  (ids -1,-2,-3)
    addSuperTriangle(pts, 100.0f);

    // Build a lookup from id -> Point for fast access during insertion
    std::unordered_map<int, const Point*> ptById;
    for (const auto& p : pts)
        ptById[p.id] = &p;

    // --- Triangulation starts with the single super-triangle ---
    struct Tri
    {
        Triangle    t;
        Circumcircle cc;
    };

    auto makeTri = [&](int a, int b, int c) -> Tri
    {
        const Point& pa = *ptById[a];
        const Point& pb = *ptById[b];
        const Point& pc = *ptById[c];
        return { Triangle(a, b, c), computeCircumcircle(pa, pb, pc) };
    };

    std::vector<Tri> tris;
    tris.reserve(vertices.size() * 3);
    tris.push_back(makeTri(-1, -2, -3));

    // --- Insert each real point one at a time ---
    for (const auto& [id, v] : vertices)
    {
        // 1. Find every triangle whose circumcircle contains this point
        std::vector<Triangle::Edge> boundary;   // edges of the polygonal hole
        std::vector<bool>          toRemove(tris.size(), false);

        for (std::size_t i = 0; i < tris.size(); ++i)
        {
            if (inCircumcircle(tris[i].cc, v.getX(), v.getY()))
            {
                toRemove[i] = true;

                // Collect the three edges of this bad triangle
                boundary.emplace_back(tris[i].t.v1, tris[i].t.v2);
                boundary.emplace_back(tris[i].t.v2, tris[i].t.v3);
                boundary.emplace_back(tris[i].t.v3, tris[i].t.v1);
            }
        }

        // 2. Remove bad triangles
        {
            std::vector<Tri> kept;
            kept.reserve(tris.size());
            for (std::size_t i = 0; i < tris.size(); ++i)
                if (!toRemove[i])
                    kept.push_back(tris[i]);
            tris = std::move(kept);
        }

        // 3. Find the boundary of the polygonal hole:
        //    edges that appear exactly once are on the boundary.
        std::vector<Triangle::Edge> hole;
        for (const auto& e : boundary)
        {
            int count = 0;
            for (const auto& f : boundary)
                if (e == f) ++count;
            if (count == 1)
                hole.push_back(e);
        }

        // 4. Re-triangulate the hole from the new point
        for (const auto& e : hole)
            tris.push_back(makeTri(id, e.a, e.b));
    }

    // --- Remove triangles that share a vertex with the super-triangle ---
    std::vector<Triangle> result;
    result.reserve(tris.size());

    for (const auto& tri : tris)
        if (!tri.t.hasSuperVertex())
            result.push_back(tri.t);

    return result;
}

// ---------------------------------------------------------------------------
// Build a graph that contains all original vertices plus triangulation edges
// ---------------------------------------------------------------------------

Graph DelaunayTriangulation::buildTriangulatedGraph(const Graph& graph)
{
    Graph result;

    // Copy vertices
    for (const auto& [id, v] : graph.getVertices())
        result.addVertexWithID(id, v.getX(), v.getY(), v.getName());

    const std::vector<Triangle> tris = triangulate(graph);

    // Use a set to avoid duplicate edges (each interior edge is shared by 2 triangles)
    using EdgeKey = std::pair<int, int>;
    std::set<EdgeKey> added;

    auto tryAdd = [&](int a, int b)
    {
        int lo = std::min(a, b);
        int hi = std::max(a, b);
        EdgeKey key { lo, hi };
        if (added.count(key)) return;
        added.insert(key);

        // Weight = Euclidean distance
        const Vertex& va = graph.getVertex(a);
        const Vertex& vb = graph.getVertex(b);
        float dx = va.getX() - vb.getX();
        float dy = va.getY() - vb.getY();
        float dist = std::sqrt(dx * dx + dy * dy);
        result.addEdge(a, b, dist);
    };

    for (const auto& tri : tris)
    {
        tryAdd(tri.v1, tri.v2);
        tryAdd(tri.v2, tri.v3);
        tryAdd(tri.v3, tri.v1);
    }

    return result;
}
