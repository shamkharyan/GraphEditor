#ifndef GRAPH_EDITOR_H
#define GRAPH_EDITOR_H

#include "model/Graph.h"

class GraphEditor
{
public:
    static GraphEditor& instance(int argc, char *argv[]);
    void execute();
private:
    GraphEditor(int argc, char *argv[]);
    GraphEditor(const GraphEditor&) = delete;
    GraphEditor(GraphEditor&&) noexcept = delete;
    GraphEditor& operator=(const GraphEditor&) = delete;
    GraphEditor& operator=(GraphEditor&&) noexcept = delete;
private:
    Graph m_graph;
};

#endif // GRAPH_EDITOR_H
