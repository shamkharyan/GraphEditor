#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "gridscene.h"
#include "gridgraphicsview.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , gridGraphicsView(new GridGraphicsView)
    , gridScene(new GridScene)
{
    ui->setupUi(this);

    QActionGroup *actionGroup_Tools = new QActionGroup(this);
    actionGroup_Tools->addAction(ui->actionSelect);
    actionGroup_Tools->addAction(ui->actionAdd_Node);
    actionGroup_Tools->addAction(ui->actionAdd_Edge);
    actionGroup_Tools->setExclusive(true);

    gridGraphicsView->setDragMode(QGraphicsView::RubberBandDrag);
    gridGraphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    gridGraphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    constexpr int sizeLimit = 20000;

    gridScene->setSceneRect(-sizeLimit, -sizeLimit, sizeLimit, sizeLimit);

    gridGraphicsView->setScene(gridScene);
    ui->centralwidget->layout()->addWidget(gridGraphicsView);

    resizeDocks({ui->dockAction_History}, {120}, Qt::Vertical);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionFullscreen_triggered(bool checked)
{
    if (checked)
        showFullScreen();
    else
        showNormal();
}


void MainWindow::on_dockAction_History_visibilityChanged(bool visible)
{
    if (visible)
        ui->actionShow_Action_History->setChecked(true);
    else
        ui->actionShow_Action_History->setChecked(false);
}



void MainWindow::on_actionShow_Action_History_triggered(bool checked)
{
    if (checked)
        ui->dockAction_History->show();
    else
        ui->dockAction_History->hide();
}


void MainWindow::on_actionZoom_In_triggered()
{
    constexpr double scaleFactor = 1.15;

    gridGraphicsView->zoom(scaleFactor);
}


void MainWindow::on_actionZoom_Out_triggered()
{
    constexpr double scaleFactor = 1.0 / 1.15;

    gridGraphicsView->zoom(scaleFactor);
}


void MainWindow::on_actionReset_Zoom_triggered()
{
    gridGraphicsView->resetTransform();
}


void MainWindow::on_actionSelect_triggered()
{
    gridGraphicsView->setDragMode(QGraphicsView::RubberBandDrag);
    gridGraphicsView->setMode(GridGraphicsView::Mode::Select);
}


void MainWindow::on_actionAdd_Node_triggered()
{
    gridGraphicsView->setDragMode(QGraphicsView::NoDrag);
    gridGraphicsView->setCursor(Qt::CrossCursor);
    gridGraphicsView->setMode(GridGraphicsView::Mode::AddNode);
}


void MainWindow::on_actionAdd_Edge_triggered()
{
    gridGraphicsView->setDragMode(QGraphicsView::NoDrag);
    gridGraphicsView->setCursor(Qt::CrossCursor);
    gridGraphicsView->setMode(GridGraphicsView::Mode::AddEdge);
}


void MainWindow::on_actionShow_Grid_triggered(bool checked)
{
    gridScene->setShowGrid(checked);
    gridScene->update();
}


void MainWindow::on_actionShow_Toolbar_triggered(bool checked)
{
    ui->toolBar->setVisible(checked);
}

