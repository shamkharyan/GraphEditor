#include "view/EdgeItem.h"
#include "view/VertexItem.h"
#include "GridScene.h"
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QInputDialog>
#include <QtMath>
#include <QDebug>

EdgeItem::EdgeItem(int id, VertexItem* start, VertexItem* end, float weight, QGraphicsItem* parent)
    : QGraphicsLineItem(parent)
    , m_edgeId(id)
    , m_startVertex(start)
    , m_endVertex(end)
    , m_weight(weight)
    , m_weightLabel(nullptr)
{
    if (!start || !end)
    {
        qWarning() << "EdgeItem created with null vertex!";
        return;
    }

    setPen(QPen(Qt::black, 2));
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);

    m_weightLabel = new QGraphicsTextItem(this);
    if (!m_weightLabel)
    {
        qWarning() << "Failed to create weight label!";
        return;
    }

    m_weightLabel->setPlainText(QString::number(weight, 'g', 3));
    m_weightLabel->setDefaultTextColor(Qt::darkBlue);
    m_weightLabel->setFlag(QGraphicsItem::ItemIsSelectable, false);

    QFont font = m_weightLabel->font();
    font.setPointSize(9);
    m_weightLabel->setFont(font);

    updatePosition();
}

void EdgeItem::updatePosition()
{
    if (!m_startVertex || !m_endVertex)
        return;

    QPointF p1 = m_startVertex->sceneBoundingRect().center();
    QPointF p2 = m_endVertex->sceneBoundingRect().center();
    setLine(QLineF(p1, p2));

    if (m_weightLabel)
    {
        QPointF mid = (p1 + p2) / 2;
        QRectF labelRect = m_weightLabel->boundingRect();
        m_weightLabel->setPos(mid.x() - labelRect.width() / 2, mid.y() - labelRect.height() / 2);
    }
}

void EdgeItem::setWeight(float weight)
{
    m_weight = weight;

    if (!m_weightLabel)
        return;

    m_weightLabel->setPlainText(QString::number(weight, 'g', 3));

    QPointF mid = (line().p1() + line().p2()) / 2;
    QRectF labelRect = m_weightLabel->boundingRect();
    m_weightLabel->setPos(mid.x() - labelRect.width() / 2, mid.y() - labelRect.height() / 2);
}

void EdgeItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setRenderHint(QPainter::Antialiasing);

    QPen p = pen();
    if (isSelected())
    {
        p.setColor(Qt::red);
        p.setWidth(3);
    }
    painter->setPen(p);
    painter->drawLine(line());

    // Draw arrow
    QLineF l = line();
    double angle = std::atan2(-l.dy(), l.dx());

    QPointF arrowP1 = l.p2() - QPointF(sin(angle + M_PI / 3) * 10,
                                       cos(angle + M_PI / 3) * 10);
    QPointF arrowP2 = l.p2() - QPointF(sin(angle + M_PI - M_PI / 3) * 10,
                                       cos(angle + M_PI - M_PI / 3) * 10);

    QPolygonF arrowHead;
    arrowHead << l.p2() << arrowP1 << arrowP2;
    painter->setBrush(p.color());
    painter->drawPolygon(arrowHead);
}

void EdgeItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    Q_UNUSED(event);

    bool ok;
    double newWeight = QInputDialog::getDouble(
        nullptr,
        "Edit Edge Weight",
        "Enter new weight:",
        m_weight,
        -1000.0,
        1000.0,
        2,
        &ok
        );

    if (ok)
    {
        setWeight(newWeight);

        GridScene* gridScene = dynamic_cast<GridScene*>(scene());
        if (gridScene)
        {
            gridScene->getGraph().reweightEdge(m_edgeId, newWeight);
            emit gridScene->actionLogged(QString("Changed edge %1 weight to %2").arg(m_edgeId).arg(newWeight));
        }
    }
}
