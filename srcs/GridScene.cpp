#include "GridScene.h"
#include "view/VertexItem.h"
#include "view/EdgeItem.h"
#include "model/Vertex.h"
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QInputDialog>
#include <QGraphicsView>
#include <QPainter>
#include <QDebug>
#include <QtMath>
#include <QMessageBox>
#include <unordered_set>
#include <cmath>

// ============================================================================
// GridScene Implementation
// ============================================================================

GridScene::GridScene(QObject* parent)
    : QGraphicsScene(parent)
{
    setItemIndexMethod(QGraphicsScene::NoIndex);
    m_vertexItems.reserve(1000);
    m_edgeItems.reserve(2000);
}

void GridScene::setShowGrid(bool value)
{
    m_showGrid = value;
    update();
}

void GridScene::setMode(Mode mode)
{
    m_mode = mode;
    clearEdgePreview();
}

void GridScene::drawBackground(QPainter* painter, const QRectF& rect)
{
    if (m_showGrid)
    {
        QTransform t = views().isEmpty() ? QTransform() : views().first()->transform();
        double scaleFactor = t.m11();
        double gridSize;

        if      (scaleFactor > 4.0) gridSize = 5;
        else if (scaleFactor > 3.0) gridSize = 10;
        else if (scaleFactor > 2.0) gridSize = 20;
        else if (scaleFactor > 1.0) gridSize = 50;
        else                        gridSize = 100;

        QPen pen(QColor(160, 160, 160), 0);
        painter->setPen(pen);

        double startX = rect.left()  - fmod(rect.left(),  gridSize);
        double startY = rect.top()   - fmod(rect.top(),   gridSize);

        for (double x = startX; x < rect.right();  x += gridSize)
            painter->drawLine(QPointF(x, rect.top()), QPointF(x, rect.bottom()));

        for (double y = startY; y < rect.bottom(); y += gridSize)
            painter->drawLine(QPointF(rect.left(), y), QPointF(rect.right(), y));
    }
}

void GridScene::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() != Qt::LeftButton)
    {
        QGraphicsScene::mousePressEvent(event);
        return;
    }

    QPointF pos = event->scenePos();

    switch (m_mode)
    {
    case Mode::AddNode:
        addNodeAt(pos);
        event->accept();
        break;

    case Mode::AddEdge:
    {
        VertexItem* vertexItem = nullptr;
        for (QGraphicsItem* item : items(pos))
        {
            if (auto* v = dynamic_cast<VertexItem*>(item))
            {
                vertexItem = v;
                break;
            }
        }

        if (vertexItem)
        {
            if (!m_edgeStartVertex)
                startEdgeFrom(vertexItem);
            else
                completeEdgeTo(vertexItem);
            event->accept();
        }
        else
        {
            clearEdgePreview();
            event->accept();
        }
        break;
    }

    case Mode::Select:
    default:
        QGraphicsScene::mousePressEvent(event);
        break;
    }
}

void GridScene::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (m_mode == Mode::AddEdge && m_edgePreviewLine && m_edgeStartVertex)
    {
        QPointF startPos = m_edgeStartVertex->sceneBoundingRect().center();
        m_edgePreviewLine->setLine(QLineF(startPos, event->scenePos()));
        event->accept();
    }
    else
    {
        QGraphicsScene::mouseMoveEvent(event);
    }
}

void GridScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsScene::mouseReleaseEvent(event);
}

void GridScene::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace)
    {
        deleteSelected();
        event->accept();
    }
    else if (event->key() == Qt::Key_Escape)
    {
        clearEdgePreview();
        clearSelection();
        event->accept();
    }
    else
    {
        QGraphicsScene::keyPressEvent(event);
    }
}

void GridScene::addNodeAt(const QPointF& pos)
{
    int id = m_graph.addVertex(pos.x(), pos.y());
    VertexItem* item = new VertexItem(id, pos.x(), pos.y(), QString::number(id));
    addItem(item);
    m_vertexItems[id] = item;
    emit actionLogged(QString("Added vertex %1 at (%2, %3)")
                          .arg(id).arg(pos.x(), 0, 'f', 1).arg(pos.y(), 0, 'f', 1));
    emit graphChanged();
}

void GridScene::startEdgeFrom(VertexItem* vertex)
{
    m_edgeStartVertex = vertex;

    QPointF startPos = vertex->sceneBoundingRect().center();
    m_edgePreviewLine = new QGraphicsLineItem(QLineF(startPos, startPos));
    QPen pen(Qt::gray);
    pen.setStyle(Qt::DashLine);
    pen.setWidth(2);
    m_edgePreviewLine->setPen(pen);
    m_edgePreviewLine->setZValue(-1);
    addItem(m_edgePreviewLine);

    emit actionLogged(QString("Started edge from vertex %1").arg(vertex->getVertexId()));
}

void GridScene::completeEdgeTo(VertexItem* vertex)
{
    if (!m_edgeStartVertex || !vertex)
    {
        clearEdgePreview();
        emit actionLogged("Error: Invalid vertex selection");
        return;
    }

    if (m_edgeStartVertex == vertex)
    {
        clearEdgePreview();
        emit actionLogged("Cannot create self-loop");
        return;
    }

    int startId = m_edgeStartVertex->getVertexId();
    int endId   = vertex->getVertexId();

    // Default weight = Euclidean distance
    const Vertex& va = m_graph.getVertex(startId);
    const Vertex& vb = m_graph.getVertex(endId);
    float dx = va.getX() - vb.getX();
    float dy = va.getY() - vb.getY();
    double defaultWeight = std::sqrt((double)(dx * dx + dy * dy));

    // Parent the dialog to the main window so it centers there, not randomly
    QWidget* parentWidget = views().isEmpty() ? nullptr : views().first()->window();
    bool ok;
    double weight = QInputDialog::getDouble(
        parentWidget, "Edge Weight", "Enter edge weight:",
        defaultWeight, -1000000.0, 1000000.0, 2, &ok);

    if (ok)
    {
        try
        {
            int edgeId = m_graph.addEdge(startId, endId, static_cast<float>(weight));
            EdgeItem* edgeItem = new EdgeItem(edgeId, m_edgeStartVertex, vertex, static_cast<float>(weight));
            addItem(edgeItem);
            edgeItem->setZValue(-2);
            m_edgeItems[edgeId] = edgeItem;
            emit actionLogged(QString("Added edge %1: %2 → %3 (weight: %4)")
                                  .arg(edgeId).arg(startId).arg(endId).arg(weight));
            emit graphChanged();
        }
        catch (const std::exception& e)
        {
            emit actionLogged(QString("Error: %1").arg(e.what()));
        }
    }

    clearEdgePreview();
}

void GridScene::clearEdgePreview()
{
    if (m_edgePreviewLine)
    {
        removeItem(m_edgePreviewLine);
        delete m_edgePreviewLine;
        m_edgePreviewLine = nullptr;
    }
    m_edgeStartVertex = nullptr;
}

void GridScene::updateGraph()
{
    for (auto& [id, vertexItem] : m_vertexItems)
    {
        QPointF center = vertexItem->sceneBoundingRect().center();
        m_graph.moveVertex(id, center.x(), center.y());
    }
    for (auto& [id, edgeItem] : m_edgeItems)
        edgeItem->updatePosition();
}

void GridScene::deleteSelected()
{
    QList<QGraphicsItem*> selected = selectedItems();
    if (selected.isEmpty())
        return;

    // Pass 1: collect IDs from selection
    std::unordered_set<int> vertexIds;
    std::unordered_set<int> edgeIds;

    for (QGraphicsItem* item : selected)
    {
        if (auto* v = dynamic_cast<VertexItem*>(item))
            vertexIds.insert(v->getVertexId());
        else if (auto* e = dynamic_cast<EdgeItem*>(item))
            edgeIds.insert(e->getEdgeId());
    }

    // Pass 2: pull in all edges connected to any selected vertex
    for (int vid : vertexIds)
    {
        auto it = m_graph.getConnectedEdgesIds().find(vid);
        if (it != m_graph.getConnectedEdgesIds().end())
            for (int eid : it->second)
                edgeIds.insert(eid);
    }

    // Pass 3: remove edge graphics + model (each id exactly once, no double-free)
    for (int eid : edgeIds)
    {
        auto it = m_edgeItems.find(eid);
        if (it != m_edgeItems.end())
        {
            removeItem(it->second);
            delete it->second;
            m_edgeItems.erase(it);
        }
        m_graph.removeEdge(eid);
    }

    // Pass 4: remove vertex graphics + model
    for (int vid : vertexIds)
    {
        auto it = m_vertexItems.find(vid);
        if (it != m_vertexItems.end())
        {
            removeItem(it->second);
            delete it->second;
            m_vertexItems.erase(it);
        }
        m_graph.removeVertex(vid);
    }

    QStringList log;
    if (!vertexIds.empty()) log << QString("%1 vertices").arg((int)vertexIds.size());
    if (!edgeIds.empty())   log << QString("%1 edges").arg((int)edgeIds.size());
    emit actionLogged("Deleted: " + log.join(", "));
    emit graphChanged();
}

void GridScene::drawGraph(const Graph& graph)
{
    // Block Qt's internal change-notifications during bulk rebuild for performance.
    // We manually emit the necessary signals after.
    blockSignals(true);

    clear();
    m_vertexItems.clear();
    m_edgeItems.clear();
    m_graph.clear();

    const auto& vertices = graph.getVertices();
    const auto& edges    = graph.getEdges();
    m_vertexItems.reserve(vertices.size());
    m_edgeItems.reserve(edges.size());

    for (const auto& [id, vertex] : vertices)
    {
        m_graph.addVertexWithID(id, vertex.getX(), vertex.getY(), vertex.getName());
        QString label = vertex.getName().empty()
                            ? QString::number(id)
                            : QString::fromStdString(vertex.getName());
        VertexItem* vi = new VertexItem(id, vertex.getX(), vertex.getY(), label);
        addItem(vi);
        m_vertexItems[id] = vi;
    }

    for (const auto& [id, edge] : edges)
    {
        m_graph.addEdgeWithID(id,
                              edge.getStartVertexID(),
                              edge.getEndVertexID(),
                              edge.getWeight(),
                              edge.getName());

        auto sit = m_vertexItems.find(edge.getStartVertexID());
        auto eit = m_vertexItems.find(edge.getEndVertexID());
        if (sit != m_vertexItems.end() && eit != m_vertexItems.end())
        {
            EdgeItem* ei = new EdgeItem(id, sit->second, eit->second, edge.getWeight());
            addItem(ei);
            ei->setZValue(-2);
            m_edgeItems[id] = ei;
        }
    }

    blockSignals(false);
    // Force a full repaint of the scene
    invalidate(sceneRect(), QGraphicsScene::AllLayers);

    emit actionLogged(QString("Graph loaded: %1 vertices, %2 edges")
                          .arg((int)vertices.size()).arg((int)edges.size()));
    emit graphChanged();
}
