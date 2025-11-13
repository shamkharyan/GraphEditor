#include "GridScene.h"
#include <QtMath>
#include <QDebug>
#include <QGraphicsView>
#include <QGraphicsTextItem>

GridScene::GridScene(QObject *parent)
    : QGraphicsScene(parent)
{
}

void GridScene::setShowGrid(bool value)
{
    m_showGrid = value;
}

void GridScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    if (m_showGrid)
    {
        QTransform t = views().isEmpty() ? QTransform() : views().first()->transform();
        double scaleFactor = t.m11();

        double gridSize;

        if (scaleFactor > 4.0)
            gridSize = 5;
        else if (scaleFactor > 3.0)
            gridSize = 10;
        else if (scaleFactor > 2.0)
            gridSize = 20;
        else if (scaleFactor > 1.0)
            gridSize = 50;
        else
            gridSize = 100;

        QPen pen(QColor(80, 80, 80), 0);
        painter->setPen(pen);

        double startX = rect.left() - fmod(rect.left(), gridSize);
        double startY = rect.top() - fmod(rect.top(), gridSize);

        qDebug() << startX << ' ' << startY;

        for (double x = startX; x < rect.right(); x += gridSize)
            painter->drawLine(QPointF(x, rect.top()), QPointF(x, rect.bottom()));

        for (double y = startY; y < rect.bottom(); y += gridSize)
            painter->drawLine(QPointF(rect.left(), y), QPointF(rect.right(), y));
    }
}

void GridScene::drawGraph(const Graph& graph)
{
    clear(); // Remove any previous items before drawing

    // 🎨 Style setup
    QPen edgePen(Qt::black);
    edgePen.setWidth(2);

    QBrush vertexBrush(QColor(100, 180, 255)); // blueish
    QPen vertexPen(Qt::black);
    vertexPen.setWidth(1);

    const int radius = 10;

    // 🕸 Draw edges first (so they appear under vertices)
    for (const auto& edge : graph.getEdges())
    {
        const auto& vertices = graph.getVertices();
        if (vertices.find(edge.second.getStartVertexID()) == vertices.end() ||
            vertices.find(edge.second.getEndVertexID()) == vertices.end())
            continue;

        const auto& v1 = vertices.at(edge.second.getStartVertexID());
        const auto& v2 = vertices.at(edge.second.getEndVertexID());

        addLine(v1.getX(), v1.getY(), v2.getX(), v2.getY(), edgePen);
    }

    // ⚫ Draw vertices
    for (const auto& [id, vertex] : graph.getVertices())
    {
        auto circle = addEllipse(
            vertex.getX() - radius,
            vertex.getY() - radius,
            radius * 2,
            radius * 2,
            vertexPen,
            vertexBrush
            );

        // Optional: make vertex identifiable by text label (ID)
        auto label = addText(QString::number(id));
        label->setDefaultTextColor(Qt::black);
        label->setPos(vertex.getX() + radius + 2, vertex.getY() - radius - 2);
    }
}

