#ifndef GRIDGRAPHICSVIEW_H
#define GRIDGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QMouseEvent>
#include <QWheelEvent>

class GridGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    enum class Mode
    {
        Select,
        AddNode,
        AddEdge
    };

    explicit GridGraphicsView(QWidget* parent = nullptr);
    void zoom(double scaleFactor);
    void setMode(Mode mode);

protected:
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    Mode m_mode = Mode::Select;
    bool m_middleMousePressed = false;
    QPoint m_lastMousePos;
};

#endif // GRIDGRAPHICSVIEW_H
