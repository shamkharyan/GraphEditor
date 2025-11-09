#include "GridGraphicsView.h"
#include <QWheelEvent>
#include <QGraphicsEllipseItem>
#include <QDebug>
#include <QScrollBar>

GridGraphicsView::GridGraphicsView(QWidget* parent) : QGraphicsView(parent) {}

void GridGraphicsView::zoom(double scaleFactor)
{
    constexpr double maxScale = 5;
    constexpr double minScale = 0.2;

    double currentScale = transform().m11();
    if (scaleFactor > 1 && currentScale < maxScale)
    {
        scale(scaleFactor, scaleFactor);
    }
    else if (scaleFactor < 1 && currentScale > minScale)
    {
        scale(scaleFactor, scaleFactor);
    }
}

void GridGraphicsView::setMode(Mode mode)
{
    m_mode = mode;
}

void GridGraphicsView::wheelEvent(QWheelEvent* event)
{
    constexpr double scaleFactor = 1.15;

    if (event->angleDelta().y() > 0)
        zoom(scaleFactor);
    else if (event->angleDelta().y() < 0)
        zoom(1.0 / scaleFactor);
    event->accept();
}

void GridGraphicsView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton)
    {
        m_middleMousePressed = true;
        m_lastMousePos = event->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
    }
    else
    {
        QGraphicsView::mousePressEvent(event);
    }
}

void GridGraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    if (m_middleMousePressed)
    {
        QPoint delta = event->pos() - m_lastMousePos;
        m_lastMousePos = event->pos();

        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());

        event->accept();
    }
    else
    {
        QGraphicsView::mouseMoveEvent(event);
    }
}

void GridGraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton)
    {
        m_middleMousePressed = false;
        if (m_mode == Mode::Select)
            unsetCursor();
        else
            setCursor(Qt::CrossCursor);

        event->accept();
    }
    else
    {
        QGraphicsView::mouseReleaseEvent(event);
    }
}


