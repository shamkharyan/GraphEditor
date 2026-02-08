#ifndef GRIDSCENE_H
#define GRIDSCENE_H

#include <QGraphicsScene>
#include <QPainter>
#include "model/Graph.h"

class VertexItem;
class EdgeItem;

class GridScene : public QGraphicsScene
{
    Q_OBJECT
public:
    enum class Mode
    {
        Select,
        AddNode,
        AddEdge
    };

    explicit GridScene(QObject* parent = nullptr);
    void setShowGrid(bool value);
    void drawGraph(const Graph& graph);
    void setMode(Mode mode);

    Graph& getGraph() { return m_graph; }
    const Graph& getGraph() const { return m_graph; }

signals:
    void graphChanged();
    void actionLogged(const QString& action);

protected:
    void drawBackground(QPainter* painter, const QRectF& rect) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    void addNodeAt(const QPointF& pos);
    void startEdgeFrom(VertexItem* vertex);
    void completeEdgeTo(VertexItem* vertex);
    void clearEdgePreview();
    void updateGraph();
    void deleteSelected();

    bool m_showGrid = true;
    Mode m_mode = Mode::Select;
    Graph m_graph;

    VertexItem* m_edgeStartVertex = nullptr;
    QGraphicsLineItem* m_edgePreviewLine = nullptr;

    std::unordered_map<int, VertexItem*> m_vertexItems;
    std::unordered_map<int, EdgeItem*> m_edgeItems;
};

#endif // GRIDSCENE_H
