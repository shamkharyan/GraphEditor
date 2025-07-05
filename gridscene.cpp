#include "gridscene.h"
#include <QtMath>
#include <QDebug>
#include <QGraphicsView>

GridScene::GridScene(QObject *parent)
    : QGraphicsScene(parent)
{
}

void GridScene::setShowGrid(bool value)
{
    m_showGrid = value;
}

void GridScene::drawBackground(QPainter *painter, const QRectF &rect)
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

        qDebug() << startX << ' ' << startY;

        for (double x = startX; x < rect.right(); x += gridSize)
            painter->drawLine(QPointF(x, rect.top()), QPointF(x, rect.bottom()));

        for (double y = startY; y < rect.bottom(); y += gridSize)
            painter->drawLine(QPointF(rect.left(), y), QPointF(rect.right(), y));
    }
}

