#ifndef GRID_SCENE_H
#define GRID_SCENE_H

#include <QGraphicsScene>
#include <QPainter>
#include <QMouseEvent>

#include "model/Graph.h"

class GridScene : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit GridScene(QObject *parent = nullptr);

    void setShowGrid(bool value);
    void drawGraph(const Graph& graph);

protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override;

private:
    bool m_showGrid = true;
};

#endif // GRID_SCENE_H
