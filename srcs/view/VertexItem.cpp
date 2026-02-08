#include "view/VertexItem.h"
#include "GridScene.h"
#include "view/EdgeItem.h"
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QInputDialog>

VertexItem::VertexItem(int id, float x, float y, const QString& label, QGraphicsItem* parent)
    : QGraphicsEllipseItem(-15, -15, 30, 30, parent)
    , m_vertexId(id)
{
    setPos(x, y);
    setBrush(QBrush(QColor(100, 180, 255)));
    setPen(QPen(Qt::black, 2));
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    setFlag(QGraphicsItem::ItemClipsToShape, true);
    setAcceptHoverEvents(true);
    setZValue(1);
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);

    m_label = new QGraphicsTextItem(label, this);
    m_label->setDefaultTextColor(Qt::black);
    m_label->setFlag(QGraphicsItem::ItemIsSelectable, false);
    m_label->setFlag(QGraphicsItem::ItemIsMovable, false);
    QFont font = m_label->font();
    font.setPointSize(10);
    font.setBold(true);
    m_label->setFont(font);

    QRectF labelRect = m_label->boundingRect();
    m_label->setPos(-labelRect.width() / 2, -labelRect.height() / 2);
}

void VertexItem::updateLabel(const QString& label)
{
    m_label->setPlainText(label);
    QRectF labelRect = m_label->boundingRect();
    m_label->setPos(-labelRect.width() / 2, -labelRect.height() / 2);
}

QVariant VertexItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
    if (change == ItemPositionHasChanged && scene())
    {
        GridScene* gridScene = dynamic_cast<GridScene*>(scene());
        if (gridScene)
        {
            QPointF center = sceneBoundingRect().center();
            gridScene->getGraph().moveVertex(m_vertexId, center.x(), center.y());

            // Update only connected edges efficiently
            const auto& connectedEdges = gridScene->getGraph().getConnectedEdgesIds();
            auto it = connectedEdges.find(m_vertexId);
            if (it != connectedEdges.end())
            {
                for (int edgeId : it->second)
                {
                    // Find edge item and update its position
                    foreach (QGraphicsItem* item, scene()->items())
                    {
                        EdgeItem* edge = dynamic_cast<EdgeItem*>(item);
                        if (edge && edge->getEdgeId() == edgeId)
                        {
                            edge->updatePosition();
                        }
                    }
                }
            }
        }
    }

    return QGraphicsEllipseItem::itemChange(change, value);
}

void VertexItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setRenderHint(QPainter::Antialiasing);

    if (isSelected())
    {
        painter->setPen(QPen(Qt::yellow, 5));
        painter->setBrush(brush());
        painter->drawEllipse(rect().adjusted(-2, -2, 2, 2));
    }
    else if (m_isHovered)
    {
        painter->setPen(QPen(QColor(150, 200, 255), 3));
        painter->setBrush(brush());
        painter->drawEllipse(rect());
    }
    else
    {
        painter->setPen(pen());
        painter->setBrush(brush());
        painter->drawEllipse(rect());
    }
}

void VertexItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    Q_UNUSED(event);

    bool ok;
    QString text = QInputDialog::getText(
        nullptr,
        "Rename Vertex",
        "Enter new label:",
        QLineEdit::Normal,
        m_label->toPlainText(),
        &ok
        );

    if (ok && !text.isEmpty())
    {
        updateLabel(text);

        GridScene* gridScene = dynamic_cast<GridScene*>(scene());
        if (gridScene)
        {
            gridScene->getGraph().renameVertex(m_vertexId, text.toStdString());
            emit gridScene->actionLogged(QString("Renamed vertex %1 to '%2'").arg(m_vertexId).arg(text));
        }
    }
}
