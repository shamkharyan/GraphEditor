#ifndef VERTEXITEM_H
#define VERTEXITEM_H

#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>

class VertexItem : public QGraphicsEllipseItem
{
public:
    VertexItem(int id, float x, float y, const QString& label, QGraphicsItem* parent = nullptr);

    int getVertexId() const { return m_vertexId; }
    void updateLabel(const QString& label);

    enum { Type = UserType + 1 };
    int type() const override { return Type; }

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;

private:
    int m_vertexId;
    QGraphicsTextItem* m_label;
    bool m_isHovered = false;
};

#endif // VERTEXITEM_H
