#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "gridgraphicsview.h"
#include "gridscene.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionFullscreen_triggered(bool checked);

    void on_dockAction_History_visibilityChanged(bool visible);

    void on_actionShow_Action_History_triggered(bool checked);

    void on_actionZoom_In_triggered();

    void on_actionZoom_Out_triggered();

    void on_actionReset_Zoom_triggered();

    void on_actionSelect_triggered();

    void on_actionAdd_Node_triggered();

    void on_actionAdd_Edge_triggered();

    void on_actionShow_Grid_triggered(bool checked);

    void on_actionShow_Toolbar_triggered(bool checked);

private:
    Ui::MainWindow *ui;

    GridGraphicsView *gridGraphicsView;
    GridScene *gridScene;
};
#endif // MAINWINDOW_H
