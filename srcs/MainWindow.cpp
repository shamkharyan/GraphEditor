#include "MainWindow.h"
#include "../ui/ui_MainWindow.h"
#include "GridScene.h"
#include "GridGraphicsView.h"
#include <QActionGroup>
#include <QFileDialog>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QInputDialog>
#include <QApplication>
#include <QPalette>
#include <QKeyEvent>
#include <QStyle>
#include <random>
#include <cmath>
#include <set>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , gridGraphicsView(new GridGraphicsView)
    , gridScene(new GridScene)
{
    ui->setupUi(this);

    // Setup tool action group for exclusive selection
    QActionGroup *actionGroup_Tools = new QActionGroup(this);
    actionGroup_Tools->addAction(ui->actionSelect);
    actionGroup_Tools->addAction(ui->actionAdd_Node);
    actionGroup_Tools->addAction(ui->actionAdd_Edge);
    actionGroup_Tools->setExclusive(true);

    // Configure graphics view
    gridGraphicsView->setDragMode(QGraphicsView::RubberBandDrag);
    gridGraphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    gridGraphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    gridGraphicsView->setRenderHint(QPainter::Antialiasing);

    // Setup scene
    constexpr int sizeLimit = 20000;
    gridScene->setSceneRect(-sizeLimit, -sizeLimit, sizeLimit * 2, sizeLimit * 2);
    gridGraphicsView->setScene(gridScene);

    // Add graphics view to central widget
    ui->centralwidget->layout()->addWidget(gridGraphicsView);

    // Setup dock sizes
    resizeDocks({ui->dockAction_History}, {120}, Qt::Vertical);

    // Connect signals
    connect(gridScene, &GridScene::actionLogged, this, &MainWindow::logAction);

    // Log startup
    logAction("Graph Editor initialized");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::logAction(const QString& action)
{
    ui->plainTextEdit->appendPlainText(action);
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
    ui->actionShow_Action_History->setChecked(visible);
}

void MainWindow::on_actionShow_Action_History_triggered(bool checked)
{
    ui->dockAction_History->setVisible(checked);
}

void MainWindow::on_actionZoom_In_triggered()
{
    constexpr double scaleFactor = 1.15;
    gridGraphicsView->zoom(scaleFactor);
    logAction("Zoomed in");
}

void MainWindow::on_actionZoom_Out_triggered()
{
    constexpr double scaleFactor = 1.0 / 1.15;
    gridGraphicsView->zoom(scaleFactor);
    logAction("Zoomed out");
}

void MainWindow::on_actionReset_Zoom_triggered()
{
    gridGraphicsView->resetTransform();
    logAction("Reset zoom");
}

void MainWindow::on_actionSelect_triggered()
{
    gridGraphicsView->setDragMode(QGraphicsView::RubberBandDrag);
    gridGraphicsView->setMode(GridGraphicsView::Mode::Select);
    gridGraphicsView->unsetCursor();
    gridScene->setMode(GridScene::Mode::Select);
    logAction("Mode: Select");
}

void MainWindow::on_actionAdd_Node_triggered()
{
    gridGraphicsView->setDragMode(QGraphicsView::NoDrag);
    gridGraphicsView->setCursor(Qt::CrossCursor);
    gridGraphicsView->setMode(GridGraphicsView::Mode::AddNode);
    gridScene->setMode(GridScene::Mode::AddNode);
    logAction("Mode: Add Node");
}

void MainWindow::on_actionAdd_Edge_triggered()
{
    gridGraphicsView->setDragMode(QGraphicsView::NoDrag);
    gridGraphicsView->setCursor(Qt::CrossCursor);
    gridGraphicsView->setMode(GridGraphicsView::Mode::AddEdge);
    gridScene->setMode(GridScene::Mode::AddEdge);
    logAction("Mode: Add Edge");
}

void MainWindow::on_actionShow_Grid_triggered(bool checked)
{
    gridScene->setShowGrid(checked);
    gridScene->update();
    logAction(checked ? "Grid shown" : "Grid hidden");
}

void MainWindow::on_actionShow_Toolbar_triggered(bool checked)
{
    ui->toolBar->setVisible(checked);
}

void MainWindow::on_actionSnap_to_Grid_triggered(bool checked)
{
    logAction(checked ? "Snap to grid enabled" : "Snap to grid disabled");
    // TODO: Implement snap to grid functionality
}

void MainWindow::on_actionDelete_triggered()
{
    // The GridScene handles deletion via keyboard
    QKeyEvent* deleteEvent = new QKeyEvent(QEvent::KeyPress, Qt::Key_Delete, Qt::NoModifier);
    QApplication::postEvent(gridScene, deleteEvent);
}

void MainWindow::on_actionGenerate_Random_Graph_triggered()
{
    bool ok;
    int numVertices = QInputDialog::getInt(
        this,
        "Generate Random Graph",
        "Number of vertices:",
        10, 3, 100, 1, &ok
        );

    if (!ok) return;

    int numEdges = QInputDialog::getInt(
        this,
        "Generate Random Graph",
        QString("Number of edges (max %1):").arg(numVertices * (numVertices - 1)),
        numVertices,
        0,
        numVertices * (numVertices - 1),
        1,
        &ok
        );

    if (!ok) return;

    // Clear current graph
    Graph newGraph;

    // Random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> disX(-500, 500);
    std::uniform_real_distribution<> disY(-500, 500);
    std::uniform_real_distribution<> disWeight(0.1, 10.0);

    // Create vertices in a circular pattern for better visualization
    const double radius = 300;
    const double angleStep = 2 * M_PI / numVertices;

    for (int i = 0; i < numVertices; ++i)
    {
        double angle = i * angleStep;
        double x = radius * cos(angle);
        double y = radius * sin(angle);
        newGraph.addVertex(x, y, std::to_string(i));
    }

    // Create random edges
    std::set<std::pair<int, int>> createdEdges;
    int edgesCreated = 0;

    std::uniform_int_distribution<> disVertex(0, numVertices - 1);

    while (edgesCreated < numEdges)
    {
        int v1 = disVertex(gen);
        int v2 = disVertex(gen);

        if (v1 == v2) continue;

        // Ensure undirected uniqueness
        auto edge = std::make_pair(std::min(v1, v2), std::max(v1, v2));

        if (createdEdges.find(edge) == createdEdges.end())
        {
            float weight = disWeight(gen);
            newGraph.addEdge(v1, v2, weight);
            createdEdges.insert(edge);
            edgesCreated++;
        }
    }

    gridScene->drawGraph(newGraph);
    logAction(QString("Generated random graph: %1 vertices, %2 edges")
                  .arg(numVertices).arg(numEdges));
}

void MainWindow::on_actionNew_File_triggered()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "New File",
        "Create a new graph? Unsaved changes will be lost.",
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes)
    {
        Graph emptyGraph;
        gridScene->drawGraph(emptyGraph);
        m_currentFilePath.clear();
        logAction("New file created");
    }
}

void MainWindow::on_actionSave_triggered()
{
    if (m_currentFilePath.isEmpty())
    {
        on_actionSave_As_triggered();
        return;
    }

    QJsonObject root;
    root["version"] = "1.0";

    // Save vertices
    QJsonArray verticesArray;
    for (const auto& [id, vertex] : gridScene->getGraph().getVertices())
    {
        QJsonObject v;
        v["id"] = id;
        v["x"] = static_cast<double>(vertex.getX());
        v["y"] = static_cast<double>(vertex.getY());
        v["name"] = QString::fromStdString(vertex.getName());
        verticesArray.append(v);
    }
    root["vertices"] = verticesArray;

    // Save edges
    QJsonArray edgesArray;
    for (const auto& [id, edge] : gridScene->getGraph().getEdges())
    {
        QJsonObject e;
        e["id"] = id;
        e["start"] = edge.getStartVertexID();
        e["end"] = edge.getEndVertexID();
        e["weight"] = static_cast<double>(edge.getWeight());
        e["name"] = QString::fromStdString(edge.getName());
        edgesArray.append(e);
    }
    root["edges"] = edgesArray;

    QJsonDocument doc(root);

    QFile file(m_currentFilePath);
    if (file.open(QIODevice::WriteOnly))
    {
        file.write(doc.toJson());
        file.close();
        logAction("Graph saved to: " + m_currentFilePath);
    }
    else
    {
        QMessageBox::warning(this, "Error", "Could not save file!");
    }
}

void MainWindow::on_actionSave_As_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "Save Graph",
        "",
        "JSON Files (*.json);;All Files (*)"
        );

    if (!fileName.isEmpty())
    {
        m_currentFilePath = fileName;
        on_actionSave_triggered();
    }
}

void MainWindow::on_actionOpen_File_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Open Graph",
        "",
        "JSON Files (*.json);;All Files (*)"
        );

    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(this, "Error", "Could not open file!");
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject())
    {
        QMessageBox::warning(this, "Error", "Invalid file format!");
        return;
    }

    QJsonObject root = doc.object();
    Graph loadedGraph;

    // Load vertices
    QJsonArray verticesArray = root["vertices"].toArray();
    for (const QJsonValue& val : verticesArray)
    {
        QJsonObject v = val.toObject();
        loadedGraph.addVertexWithID(
            v["id"].toInt(),
            v["x"].toDouble(),
            v["y"].toDouble(),
            v["name"].toString().toStdString()
            );
    }

    // Load edges
    QJsonArray edgesArray = root["edges"].toArray();
    for (const QJsonValue& val : edgesArray)
    {
        QJsonObject e = val.toObject();
        loadedGraph.addEdgeWithID(
            e["id"].toInt(),
            e["start"].toInt(),
            e["end"].toInt(),
            e["weight"].toDouble(),
            e["name"].toString().toStdString()
            );
    }

    gridScene->drawGraph(loadedGraph);
    m_currentFilePath = fileName;
    logAction("Graph loaded from: " + fileName);
}

void MainWindow::on_actionDark_Mode_triggered(bool checked)
{
    if (checked)
    {
        qApp->setStyle("Fusion");
        QPalette darkPalette;
        darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::WindowText, Qt::white);
        darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
        darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
        darkPalette.setColor(QPalette::ToolTipText, Qt::white);
        darkPalette.setColor(QPalette::Text, Qt::white);
        darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::ButtonText, Qt::white);
        darkPalette.setColor(QPalette::BrightText, Qt::red);
        darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
        darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
        darkPalette.setColor(QPalette::HighlightedText, Qt::black);
        qApp->setPalette(darkPalette);

        gridScene->setBackgroundBrush(QBrush(QColor(40, 40, 40)));
        logAction("Dark mode enabled");
    }
    else
    {
        qApp->setPalette(style()->standardPalette());
        gridScene->setBackgroundBrush(QBrush(Qt::white));
        logAction("Dark mode disabled");
    }
}
