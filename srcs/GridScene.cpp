#include "GridScene.h"
#include "model/Vertex.h"
#include "view/VertexItem.h"
#include "view/EdgeItem.h"
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QInputDialog>
#include <QGraphicsView>
#include <QPainter>
#include <QDebug>
#include <QtMath>
#include <QMessageBox>
#include <cmath>

// ============================================================================
// GridScene Implementation
// ============================================================================

GridScene::GridScene(QObject* parent)
    : QGraphicsScene(parent)
{
    setItemIndexMethod(QGraphicsScene::NoIndex); // Faster for dynamic scenes

    // Reserve space for better performance
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

        for (double x = startX; x < rect.right(); x += gridSize)
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
        // Get all items at this position
        QList<QGraphicsItem*> itemsAtPos = items(pos);
        VertexItem* vertexItem = nullptr;

        // Find the first VertexItem in the list
        for (QGraphicsItem* item : itemsAtPos)
        {
            VertexItem* candidate = dynamic_cast<VertexItem*>(item);
            if (candidate)
            {
                vertexItem = candidate;
                break;
            }
        }

        if (vertexItem)
        {
            if (!m_edgeStartVertex)
            {
                startEdgeFrom(vertexItem);
            }
            else
            {
                completeEdgeTo(vertexItem);
            }
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
        QPointF endPos = event->scenePos();
        m_edgePreviewLine->setLine(QLineF(startPos, endPos));
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

    VertexItem* vertexItem = new VertexItem(id, pos.x(), pos.y(), QString::number(id));
    addItem(vertexItem);
    m_vertexItems[id] = vertexItem;

    emit actionLogged(QString("Added vertex %1 at (%2, %3)")
                          .arg(id).arg(pos.x(), 0, 'f', 1).arg(pos.y(), 0, 'f', 1));
    emit graphChanged();
}

void GridScene::startEdgeFrom(VertexItem* vertex)
{
    m_edgeStartVertex = vertex;

    // Create preview line
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
    int endId = vertex->getVertexId();

    // Default weight = Euclidean distance between the two vertices
    const Vertex& va = m_graph.getVertex(startId);
    const Vertex& vb = m_graph.getVertex(endId);
    float dx = va.getX() - vb.getX();
    float dy = va.getY() - vb.getY();
    double defaultWeight = std::sqrt(dx * dx + dy * dy);

    bool ok;
    double weight = QInputDialog::getDouble(
        nullptr, "Edge Weight", "Enter edge weight:",
        defaultWeight, -1000000.0, 1000000.0, 2, &ok);

    if (ok)
    {
        try
        {
            int edgeId = m_graph.addEdge(startId, endId, weight);

            EdgeItem* edgeItem = new EdgeItem(edgeId, m_edgeStartVertex, vertex, weight);
            addItem(edgeItem);
            edgeItem->setZValue(-2);
            m_edgeItems[edgeId] = edgeItem;

            emit actionLogged(QString("Added edge %1 from vertex %2 to %3 (weight: %4)")
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
    // Update all vertex positions in the graph
    for (auto& [id, vertexItem] : m_vertexItems)
    {
        QPointF center = vertexItem->sceneBoundingRect().center();
        m_graph.moveVertex(id, center.x(), center.y());
    }

    // Update all edge positions
    for (auto& [id, edgeItem] : m_edgeItems)
    {
        edgeItem->updatePosition();
    }
}

void GridScene::deleteSelected()
{
    QList<QGraphicsItem*> selected = selectedItems();
    if (selected.isEmpty())
        return;

    // --- Pass 1: collect vertex IDs and explicitly selected edge IDs ---
    std::unordered_set<int> vertexIds;
    std::unordered_set<int> edgeIds;

    for (QGraphicsItem* item : selected)
    {
        if (VertexItem* v = dynamic_cast<VertexItem*>(item))
            vertexIds.insert(v->getVertexId());
        else if (EdgeItem* e = dynamic_cast<EdgeItem*>(item))
            edgeIds.insert(e->getEdgeId());
    }

    // --- Pass 2: add every edge connected to any selected vertex ---
    for (int vid : vertexIds)
    {
        auto it = m_graph.getConnectedEdgesIds().find(vid);
        if (it != m_graph.getConnectedEdgesIds().end())
            for (int eid : it->second)
                edgeIds.insert(eid);
    }

    // --- Pass 3: delete edge items (each ID handled exactly once) ---
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

    // --- Pass 4: delete vertex items ---
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
    // Disable updates during bulk operations
    //setUpdatesEnabled(false);

    // Clear existing items
    clear();
    m_vertexItems.clear();
    m_edgeItems.clear();
    m_graph.clear();

    // Reserve space based on graph size
    const auto& vertices = graph.getVertices();
    const auto& edges = graph.getEdges();

    m_vertexItems.reserve(vertices.size());
    m_edgeItems.reserve(edges.size());

    // Copy graph data and create vertex items
    for (const auto& [id, vertex] : vertices)
    {
        m_graph.addVertexWithID(id, vertex.getX(), vertex.getY(), vertex.getName());

        VertexItem* vertexItem = new VertexItem(
            id,
            vertex.getX(),
            vertex.getY(),
            QString::fromStdString(vertex.getName().empty() ? std::to_string(id) : vertex.getName())
            );
        addItem(vertexItem);
        m_vertexItems[id] = vertexItem;
    }

    // Draw edges
    for (const auto& [id, edge] : edges)
    {
        m_graph.addEdgeWithID(
            id,
            edge.getStartVertexID(),
            edge.getEndVertexID(),
            edge.getWeight(),
            edge.getName()
            );

        auto startIt = m_vertexItems.find(edge.getStartVertexID());
        auto endIt = m_vertexItems.find(edge.getEndVertexID());

        if (startIt != m_vertexItems.end() && endIt != m_vertexItems.end())
        {
            EdgeItem* edgeItem = new EdgeItem(id, startIt->second, endIt->second, edge.getWeight());
            addItem(edgeItem);
            edgeItem->setZValue(-2);
            m_edgeItems[id] = edgeItem;
        }
    }

    // Re-enable updates
    //setUpdatesEnabled(true);
    update();

    emit actionLogged(QString("Graph loaded: %1 vertices, %2 edges")
                          .arg(vertices.size()).arg(edges.size()));
    emit graphChanged();
}
