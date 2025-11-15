#include "view/VertexItem.h"

#include <QBrush>
#include <QPen>

VertexItem::VertexItem(int id, const QString& name, QGraphicsItem* parent) :
    QGraphicsEllipseItem(parent),
    m_id(id)
{
    setRect(-10, -10, 20, 20);
    setBrush(Qt::cyan);
    setPen(QPen(Qt::black, 2));

    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);

    m_label = new QGraphicsTextItem(name, this);
    m_label->setDefaultTextColor(Qt::black);
    m_label->setPos(-10, -30);
}

int VertexItem::getID() const { return m_id; }
