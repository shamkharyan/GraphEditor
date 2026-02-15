#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <functional>
#include "model/Graph.h"

namespace Ui { class MainWindow; }

class GridGraphicsView;
class GridScene;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    void logAction(const QString& action);

private slots:
    // Window
    void on_actionFullscreen_triggered(bool checked);
    void on_dockAction_History_visibilityChanged(bool visible);
    void on_actionShow_Action_History_triggered(bool checked);
    void on_actionZoom_In_triggered();
    void on_actionZoom_Out_triggered();
    void on_actionReset_Zoom_triggered();
    void on_actionShow_Grid_triggered(bool checked);
    void on_actionShow_Toolbar_triggered(bool checked);
    void on_actionSnap_to_Grid_triggered(bool checked);
    void on_actionDark_Mode_triggered(bool checked);
    void on_actionExit_triggered();
    // Tools
    void on_actionSelect_triggered();
    void on_actionAdd_Node_triggered();
    void on_actionAdd_Edge_triggered();
    // Edit
    void on_actionDelete_triggered();
    void on_actionSelect_All_triggered();
    void on_actionDeselect_All_triggered();
    void on_actionUndo_triggered();
    void on_actionRedo_triggered();
    // File
    void on_actionNew_File_triggered();
    void on_actionSave_triggered();
    void on_actionSave_As_triggered();
    void on_actionOpen_File_triggered();
    // Graph
    void on_actionGenerate_Random_Graph_triggered();
    void on_actionDelaunay_Triangulation_triggered();
    // MST
    void on_actionBuild_MST_Kruskal_triggered();
    void on_actionBuild_MST_Prim_triggered();
    void on_actionBuild_MST_Auto_triggered();
    // EMST
    void on_actionBuild_EMST_Kruskal_triggered();
    void on_actionBuild_EMST_Prim_triggered();
    void on_actionBuild_EMST_Auto_triggered();

private:
    void applyMST (std::function<Graph(const Graph&)> algo, const QString& name);
    void applyEMST(std::function<Graph(const Graph&)> algo, const QString& name);

    Ui::MainWindow*   ui;
    GridGraphicsView* gridGraphicsView;
    GridScene*        gridScene;
    QString           m_currentFilePath;
};

#endif // MAINWINDOW_H
