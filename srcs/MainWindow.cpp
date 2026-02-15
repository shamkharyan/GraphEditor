#include "MainWindow.h"
#include "../ui/ui_MainWindow.h"
#include "GridScene.h"
#include "GridGraphicsView.h"
#include "algorithms/Triangulation.h"
#include "algorithms/GraphAlgorithms.h"
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
#include <QStyle>
#include <QKeyEvent>
#include <QShortcut>
#include <QGraphicsItem>
#include <random>
#include <cmath>
#include <set>
#include <functional>

// ============================================================================
// Constructor / destructor
// ============================================================================

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , gridGraphicsView(new GridGraphicsView)
    , gridScene(new GridScene)
{
    ui->setupUi(this);

    // Exclusive tool-mode group
    QActionGroup* toolGroup = new QActionGroup(this);
    toolGroup->addAction(ui->actionSelect);
    toolGroup->addAction(ui->actionAdd_Node);
    toolGroup->addAction(ui->actionAdd_Edge);
    toolGroup->setExclusive(true);

    // Graphics view setup
    gridGraphicsView->setDragMode(QGraphicsView::RubberBandDrag);
    gridGraphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    gridGraphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    gridGraphicsView->setRenderHint(QPainter::Antialiasing);

    constexpr int sizeLimit = 20000;
    gridScene->setSceneRect(-sizeLimit, -sizeLimit, sizeLimit * 2, sizeLimit * 2);
    gridGraphicsView->setScene(gridScene);

    ui->centralwidget->layout()->addWidget(gridGraphicsView);
    resizeDocks({ui->dockAction_History}, {120}, Qt::Vertical);

    connect(gridScene, &GridScene::actionLogged, this, &MainWindow::logAction);

    // -----------------------------------------------------------------------
    // Register Ctrl+M and Ctrl+Shift+M directly on the window.
    // QAction shortcuts on sub-menu items are unreliable on Windows —
    // Qt only guarantees shortcut processing for top-level menu actions.
    // QShortcut with WindowShortcut context is always reliable.
    // -----------------------------------------------------------------------
    auto* scMST = new QShortcut(QKeySequence("Ctrl+M"), this);
    scMST->setContext(Qt::WindowShortcut);
    connect(scMST, &QShortcut::activated, this, &MainWindow::on_actionBuild_MST_Auto_triggered);

    auto* scEMST = new QShortcut(QKeySequence("Ctrl+Shift+M"), this);
    scEMST->setContext(Qt::WindowShortcut);
    connect(scEMST, &QShortcut::activated, this, &MainWindow::on_actionBuild_EMST_Auto_triggered);

    logAction("Graph Editor initialized");
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ============================================================================
// Logging
// ============================================================================

void MainWindow::logAction(const QString& action)
{
    ui->plainTextEdit->appendPlainText(action);
}

// ============================================================================
// Window / view slots
// ============================================================================

void MainWindow::on_actionFullscreen_triggered(bool checked)
{
    if (checked) showFullScreen(); else showNormal();
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
    gridGraphicsView->zoom(1.15);
    logAction("Zoomed in");
}

void MainWindow::on_actionZoom_Out_triggered()
{
    gridGraphicsView->zoom(1.0 / 1.15);
    logAction("Zoomed out");
}

void MainWindow::on_actionReset_Zoom_triggered()
{
    gridGraphicsView->resetTransform();
    logAction("Reset zoom");
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
}

void MainWindow::on_actionDark_Mode_triggered(bool checked)
{
    if (checked)
    {
        qApp->setStyle("Fusion");
        QPalette p;
        p.setColor(QPalette::Window,          QColor(53, 53, 53));
        p.setColor(QPalette::WindowText,      Qt::white);
        p.setColor(QPalette::Base,            QColor(25, 25, 25));
        p.setColor(QPalette::AlternateBase,   QColor(53, 53, 53));
        p.setColor(QPalette::ToolTipBase,     Qt::white);
        p.setColor(QPalette::ToolTipText,     Qt::white);
        p.setColor(QPalette::Text,            Qt::white);
        p.setColor(QPalette::Button,          QColor(53, 53, 53));
        p.setColor(QPalette::ButtonText,      Qt::white);
        p.setColor(QPalette::BrightText,      Qt::red);
        p.setColor(QPalette::Link,            QColor(42, 130, 218));
        p.setColor(QPalette::Highlight,       QColor(42, 130, 218));
        p.setColor(QPalette::HighlightedText, Qt::black);
        qApp->setPalette(p);
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

void MainWindow::on_actionExit_triggered()
{
    QApplication::quit();
}

// ============================================================================
// Edit / tool slots
// ============================================================================

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

void MainWindow::on_actionDelete_triggered()
{
    QKeyEvent* ev = new QKeyEvent(QEvent::KeyPress, Qt::Key_Delete, Qt::NoModifier);
    QApplication::postEvent(gridScene, ev);
}

void MainWindow::on_actionSelect_All_triggered()
{
    for (QGraphicsItem* item : gridScene->items())
        item->setSelected(true);
    logAction("Selected all items");
}

void MainWindow::on_actionDeselect_All_triggered()
{
    gridScene->clearSelection();
    logAction("Deselected all");
}

void MainWindow::on_actionUndo_triggered()  { logAction("Undo (not yet implemented)"); }
void MainWindow::on_actionRedo_triggered()  { logAction("Redo (not yet implemented)"); }

// ============================================================================
// File slots  —  JSON serialisation / deserialisation
// ============================================================================

void MainWindow::on_actionNew_File_triggered()
{
    auto reply = QMessageBox::question(this, "New File",
                                       "Create a new graph? Unsaved changes will be lost.",
                                       QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        gridScene->drawGraph(Graph{});
        m_currentFilePath.clear();
        logAction("New file created");
    }
}

// ------------------------------------------------------------------
// Serialise the current graph to a QJsonObject.
// Format:
//   {
//     "version": "1.0",
//     "vertices": [ { "id":0, "x":10.5, "y":-3.2, "name":"A" }, ... ],
//     "edges":    [ { "id":0, "start":0, "end":1, "weight":5.3, "name":"" }, ... ]
//   }
// ------------------------------------------------------------------
static QJsonObject graphToJson(const Graph& g)
{
    QJsonObject root;
    root["version"] = "1.0";

    QJsonArray vArr;
    for (const auto& [id, v] : g.getVertices())
    {
        QJsonObject o;
        o["id"]   = id;
        o["x"]    = static_cast<double>(v.getX());
        o["y"]    = static_cast<double>(v.getY());
        o["name"] = QString::fromStdString(v.getName());
        vArr.append(o);
    }
    root["vertices"] = vArr;

    QJsonArray eArr;
    for (const auto& [id, e] : g.getEdges())
    {
        QJsonObject o;
        o["id"]     = id;
        o["start"]  = e.getStartVertexID();
        o["end"]    = e.getEndVertexID();
        o["weight"] = static_cast<double>(e.getWeight());
        o["name"]   = QString::fromStdString(e.getName());
        eArr.append(o);
    }
    root["edges"] = eArr;

    return root;
}

// ------------------------------------------------------------------
// Deserialise a QJsonObject into a Graph.
// Returns an empty Graph and sets *errorOut on failure.
// ------------------------------------------------------------------
static Graph graphFromJson(const QJsonObject& root, QString* errorOut = nullptr)
{
    auto fail = [&](const QString& msg) -> Graph {
        if (errorOut) *errorOut = msg;
        return Graph{};
    };

    if (!root.contains("vertices") || !root.contains("edges"))
        return fail("Missing 'vertices' or 'edges' key.");

    Graph g;

    for (const QJsonValue& val : root["vertices"].toArray())
    {
        if (!val.isObject()) return fail("Vertex entry is not a JSON object.");
        QJsonObject v = val.toObject();
        if (!v.contains("id") || !v.contains("x") || !v.contains("y"))
            return fail("Vertex is missing required fields (id, x, y).");
        g.addVertexWithID(v["id"].toInt(),
                          static_cast<float>(v["x"].toDouble()),
                          static_cast<float>(v["y"].toDouble()),
                          v["name"].toString().toStdString());
    }

    for (const QJsonValue& val : root["edges"].toArray())
    {
        if (!val.isObject()) return fail("Edge entry is not a JSON object.");
        QJsonObject e = val.toObject();
        if (!e.contains("id") || !e.contains("start") || !e.contains("end"))
            return fail("Edge is missing required fields (id, start, end).");
        g.addEdgeWithID(e["id"].toInt(),
                        e["start"].toInt(),
                        e["end"].toInt(),
                        static_cast<float>(e["weight"].toDouble(1.0)),
                        e["name"].toString().toStdString());
    }

    return g;
}

void MainWindow::on_actionSave_triggered()
{
    if (m_currentFilePath.isEmpty())
    {
        on_actionSave_As_triggered();
        return;
    }

    QFile file(m_currentFilePath);
    if (!file.open(QIODevice::WriteOnly))
    {
        QMessageBox::warning(this, "Save Error",
                             "Could not open file for writing:\n" + m_currentFilePath);
        return;
    }

    QJsonDocument doc(graphToJson(gridScene->getGraph()));
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    logAction("Saved: " + m_currentFilePath);
}

void MainWindow::on_actionSave_As_triggered()
{
    QString fn = QFileDialog::getSaveFileName(this, "Save Graph", "",
                                              "JSON Graph Files (*.json);;All Files (*)");
    if (!fn.isEmpty())
    {
        m_currentFilePath = fn;
        on_actionSave_triggered();
    }
}

void MainWindow::on_actionOpen_File_triggered()
{
    QString fn = QFileDialog::getOpenFileName(this, "Open Graph", "",
                                              "JSON Graph Files (*.json);;All Files (*)");
    if (fn.isEmpty()) return;

    QFile file(fn);
    if (!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(this, "Open Error", "Could not read file:\n" + fn);
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseErr;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseErr);
    if (doc.isNull())
    {
        QMessageBox::warning(this, "Parse Error",
                             "JSON parse error: " + parseErr.errorString());
        return;
    }
    if (!doc.isObject())
    {
        QMessageBox::warning(this, "Format Error", "File does not contain a JSON object.");
        return;
    }

    QString loadErr;
    Graph g = graphFromJson(doc.object(), &loadErr);
    if (!loadErr.isEmpty())
    {
        QMessageBox::warning(this, "Load Error", loadErr);
        return;
    }

    gridScene->drawGraph(g);
    m_currentFilePath = fn;
    logAction("Loaded: " + fn);
}

// ============================================================================
// Graph slots
// ============================================================================

void MainWindow::on_actionGenerate_Random_Graph_triggered()
{
    bool ok;
    int nV = QInputDialog::getInt(this, "Random Graph", "Number of vertices:",
                                  10, 3, 5000, 1, &ok);
    if (!ok) return;

    int maxE = nV * (nV - 1);
    int nE   = QInputDialog::getInt(this, "Random Graph",
                                  QString("Number of edges (max %1):").arg(maxE),
                                  std::min(nV * 2, maxE), 0, maxE, 1, &ok);
    if (!ok) return;

    Graph g;
    std::mt19937 rng(std::random_device{}());
    double area = 500.0 + (nV / 10.0) * 50.0;
    std::uniform_real_distribution<double> dPos(-area, area);
    std::uniform_real_distribution<double> dW(0.1, 10.0);
    std::uniform_int_distribution<int>     dV(0, nV - 1);

    for (int i = 0; i < nV; ++i)
        g.addVertex(static_cast<float>(dPos(rng)),
                    static_cast<float>(dPos(rng)),
                    std::to_string(i));

    std::set<std::pair<int,int>> used;
    int created = 0, attempts = 0;
    while (created < nE && attempts < nE * 10)
    {
        ++attempts;
        int a = dV(rng), b = dV(rng);
        if (a == b) continue;
        if (used.insert({std::min(a,b), std::max(a,b)}).second)
        {
            g.addEdge(a, b, static_cast<float>(dW(rng)));
            ++created;
        }
    }

    gridScene->drawGraph(g);
    logAction(QString("Generated random graph: %1 vertices, %2 edges").arg(nV).arg(created));
}

void MainWindow::on_actionDelaunay_Triangulation_triggered()
{
    const Graph& cur = gridScene->getGraph();
    if (cur.getVertices().size() < 3)
    {
        QMessageBox::warning(this, "Delaunay Triangulation", "Need at least 3 vertices.");
        return;
    }
    try
    {
        Graph t = DelaunayTriangulation::buildTriangulatedGraph(cur);
        gridScene->drawGraph(t);
        logAction(QString("Delaunay triangulation: %1 vertices, %2 edges")
                      .arg((int)t.getVertices().size()).arg((int)t.getEdges().size()));
    }
    catch (const std::exception& e)
    {
        QMessageBox::critical(this, "Triangulation Error", e.what());
    }
}

// ============================================================================
// MST / EMST  private helpers
// ============================================================================

void MainWindow::applyMST(std::function<Graph(const Graph&)> algo, const QString& name)
{
    const Graph& cur = gridScene->getGraph();
    if (cur.getVertices().size() < 2)
    {
        QMessageBox::warning(this, name, "Need at least 2 vertices.");
        return;
    }
    if (cur.getEdges().empty())
    {
        QMessageBox::warning(this, name,
                             "Graph has no edges. Add edges first, or use EMST.");
        return;
    }
    try
    {
        Graph mst = algo(cur);
        gridScene->drawGraph(mst);
        logAction(QString("%1: %2 vertices, %3 edges")
                      .arg(name)
                      .arg((int)mst.getVertices().size())
                      .arg((int)mst.getEdges().size()));
    }
    catch (const std::exception& e)
    {
        QMessageBox::critical(this, name + " Error", e.what());
    }
}

void MainWindow::applyEMST(std::function<Graph(const Graph&)> mstAlgo, const QString& name)
{
    const Graph& cur = gridScene->getGraph();
    if (cur.getVertices().size() < 3)
    {
        QMessageBox::warning(this, name, "Need at least 3 vertices for EMST.");
        return;
    }

    if (!cur.getEdges().empty())
    {
        auto btn = QMessageBox::warning(
            this, name,
            QString("The graph has %1 edge(s).\n"
                    "EMST works on vertex positions only and ignores existing edges.\n"
                    "Continue?").arg((int)cur.getEdges().size()),
            QMessageBox::Ok | QMessageBox::Cancel);
        if (btn != QMessageBox::Ok) return;
    }

    try
    {
        // Build vertex-only graph (strip edges)
        Graph vOnly;
        for (const auto& [id, v] : cur.getVertices())
            vOnly.addVertexWithID(id, v.getX(), v.getY(), v.getName());

        // Delaunay triangulation → MST on triangulated graph
        Graph tri  = DelaunayTriangulation::buildTriangulatedGraph(vOnly);
        Graph emst = mstAlgo(tri);

        gridScene->drawGraph(emst);
        logAction(QString("%1: %2 vertices, %3 edges")
                      .arg(name)
                      .arg((int)emst.getVertices().size())
                      .arg((int)emst.getEdges().size()));
    }
    catch (const std::exception& e)
    {
        QMessageBox::critical(this, name + " Error", e.what());
    }
}

// ============================================================================
// MST slots
// ============================================================================

void MainWindow::on_actionBuild_MST_Kruskal_triggered()
{ applyMST(GraphAlgorithms::buildKruskalMST, "MST (Kruskal)"); }

void MainWindow::on_actionBuild_MST_Prim_triggered()
{ applyMST(GraphAlgorithms::buildPrimMST, "MST (Prim)"); }

void MainWindow::on_actionBuild_MST_Auto_triggered()
{ applyMST(GraphAlgorithms::buildAutoMST, "MST (Auto)"); }

// ============================================================================
// EMST slots
// ============================================================================

void MainWindow::on_actionBuild_EMST_Kruskal_triggered()
{ applyEMST(GraphAlgorithms::buildKruskalMST, "EMST (Kruskal)"); }

void MainWindow::on_actionBuild_EMST_Prim_triggered()
{ applyEMST(GraphAlgorithms::buildPrimMST, "EMST (Prim)"); }

void MainWindow::on_actionBuild_EMST_Auto_triggered()
{ applyEMST(GraphAlgorithms::buildAutoMST, "EMST (Auto)"); }
