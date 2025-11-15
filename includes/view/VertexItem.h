#ifndef VERTEX_ITEM_H
#define VERTEX_ITEM_H

#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QString>

class VertexItem : public QGraphicsEllipseItem
{
public:
    VertexItem(int id, const QString& name, QGraphicsItem* parent = nullptr);

    int getID() const;

private:
    int m_id;
    QGraphicsTextItem* m_label;
};

#endif // VERTEX_ITEM_H
