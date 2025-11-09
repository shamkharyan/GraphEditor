#ifndef GRID_SCENE_H
#define GRID_SCENE_H

#include <QGraphicsScene>
#include <QPainter>
#include <QMouseEvent>

class GridScene : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit GridScene(QObject *parent = nullptr);

    void setShowGrid(bool value);

protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override;

private:
    bool m_showGrid = true;
};

#endif // GRID_SCENE_H
