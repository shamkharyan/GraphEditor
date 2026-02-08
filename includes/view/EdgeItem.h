#ifndef EDGEITEM_H
#define EDGEITEM_H

#include <QGraphicsLineItem>
#include <QGraphicsTextItem>

class VertexItem;

class EdgeItem : public QGraphicsLineItem
{
public:
    EdgeItem(int id, VertexItem* start, VertexItem* end, float weight, QGraphicsItem* parent = nullptr);

    int getEdgeId() const { return m_edgeId; }
    void updatePosition();
    void setWeight(float weight);

    enum { Type = UserType + 2 };
    int type() const override { return Type; }

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;

private:
    int m_edgeId;
    VertexItem* m_startVertex;
    VertexItem* m_endVertex;
    float m_weight;
    QGraphicsTextItem* m_weightLabel = nullptr;
};

#endif // EDGEITEM_H
