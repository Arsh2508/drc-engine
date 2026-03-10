#include "MainWindow.hpp"

#include <QMenuBar>
#include <QFileDialog>
#include <QDockWidget>
#include <QMessageBox>
#include <QToolBar>
#include <QStatusBar>
#include <QLabel>

#include <rules/MinSpacingRule.hpp>
#include <rules/IntersectionRule.hpp>
#include <rules/MinWidthRule.hpp>
#include <rules/MinAreaRule.hpp>
#include <rules/EnclosureRule.hpp>
#include <rules/ContainmentRule.hpp>

// JSON loader and DRC core are used directly; this keeps Qt separate from the core.
using namespace std;

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    resize(1000, 700);

    // Use dedicated LayoutScene for rendering and violation management
    m_scene = new LayoutScene(this);
    m_view = new GraphicsView(this);
    m_view->setScene(m_scene);
    setCentralWidget(m_view);

    // Right dock for violation list
    QDockWidget* dock = new QDockWidget("Violations", this);
    m_violationList = new QListWidget(dock);
    dock->setWidget(m_violationList);
    addDockWidget(Qt::RightDockWidgetArea, dock);

        connect(m_violationList, &QListWidget::itemSelectionChanged,
            this, &MainWindow::onViolationSelected);

        buildMenu();

        // Status bar widgets
        statusBar();
        m_statusShapes = new QLabel("Shapes: 0", this);
        m_statusViolations = new QLabel("Violations: 0", this);
        statusBar()->addPermanentWidget(m_statusShapes);
        statusBar()->addPermanentWidget(m_statusViolations);
}

void MainWindow::buildMenu()
{
    QMenu* fileMenu = menuBar()->addMenu("File");
    QAction* openAct = fileMenu->addAction("Open Layout...");
    connect(openAct, &QAction::triggered, this, &MainWindow::onOpenLayout);

    QMenu* runMenu = menuBar()->addMenu("Run");
    QAction* runAct = runMenu->addAction("Execute DRC");
    connect(runAct, &QAction::triggered, this, &MainWindow::onRunDrc);

    // Toolbar
    m_toolbar = addToolBar("Main");
    QAction* tbOpen = m_toolbar->addAction("Open");
    connect(tbOpen, &QAction::triggered, this, &MainWindow::onOpenLayout);
    QAction* tbRun = m_toolbar->addAction("Run DRC");
    connect(tbRun, &QAction::triggered, this, &MainWindow::onRunDrc);
    QAction* tbReset = m_toolbar->addAction("Reset View");
    connect(tbReset, &QAction::triggered, [this]() {
        if (m_scene)
            m_scene->resetHighlight();
        if (m_view && m_scene)
        {
            m_view->resetTransform();
            QTransform t; t.scale(1.0, -1.0); m_view->setTransform(t);
            m_view->fitInView(m_scene->getLayoutBounds(), Qt::KeepAspectRatio);
        }
    });
}

void MainWindow::onOpenLayout()
{
    QString filename = QFileDialog::getOpenFileName(this, "Open JSON Layout", QString(), "JSON Files (*.json);;All Files (*)");
    if (filename.isEmpty())
        return;

    try
    {
        JsonLayoutLoader loader;
        // use loadWithRules so we also parse any rule definitions
        auto pair = loader.loadWithRules(filename.toStdString());
        m_layout = pair.first;
        m_parsedRules = pair.second;

        // simple logging for developers
        std::cout << "[GUI] loaded layout with " << m_parsedRules.size() << " rule(s) from JSON" << std::endl;
        for (const auto& r : m_parsedRules)
        {
            if (r)
                std::cout << "    rule: " << r->getDescription() << std::endl;
        }
    }
    catch (const std::exception& ex)
    {
        QMessageBox::critical(this, "Load Error", ex.what());
        return;
    }

    // Load layout into scene and fit to view
    m_scene->loadLayout(m_layout);
    m_violationList->clear();

    // Update status bar with number of shapes
    if (m_statusShapes)
        m_statusShapes->setText(QString("Shapes: %1").arg(m_layout ? (int)m_layout->getTotalShapeCount() : 0));

    // Fit the entire layout into the view
    m_view->fitInView(m_scene->getLayoutBounds(), Qt::KeepAspectRatio);
}

void MainWindow::loadLayoutFromPath(const QString& path)
{
    if (path.isEmpty())
        return;

    try
    {
        JsonLayoutLoader loader;
        m_layout = loader.load(path.toStdString());
    }
    catch (const std::exception& ex)
    {
        QMessageBox::critical(this, "Load Error", ex.what());
        return;
    }

    m_scene->loadLayout(m_layout);
    m_violationList->clear();

    if (m_statusShapes)
        m_statusShapes->setText(QString("Shapes: %1").arg(m_layout ? (int)m_layout->getTotalShapeCount() : 0));

    m_view->fitInView(m_scene->getLayoutBounds(), Qt::KeepAspectRatio);
}

void MainWindow::onRunDrc()
{
    if (!m_layout)
    {
        QMessageBox::warning(this, "No Layout", "Please open a layout first.");
        return;
    }

    // Register rules. Prefer rules parsed from JSON file if any.
    m_engine.clearRules();

    bool needIntersection = true;
    bool needSpacing = true;

    if (!m_parsedRules.empty())
    {
        for (const auto& r : m_parsedRules)
        {
            if (!r)
                continue;

            // detect if JSON already gave us these common rules
            if (dynamic_cast<IntersectionRule*>(r.get()))
                needIntersection = false;
            if (dynamic_cast<MinSpacingRule*>(r.get()))
                needSpacing = false;

            m_engine.registerRule(r);
        }
    }

    if (m_parsedRules.empty())
    {
        // fallback: register all rule types with reasonable defaults for standard layer names
        m_engine.registerRule(std::make_shared<IntersectionRule>());
        m_engine.registerRule(std::make_shared<MinSpacingRule>(10, 0));
        m_engine.registerRule(std::make_shared<MinSpacingRule>(5, 1));
        m_engine.registerRule(std::make_shared<MinWidthRule>());
        m_engine.registerRule(std::make_shared<MinAreaRule>());
        // Enclosure and Containment use standard layer names (metal1, metal2)
        m_engine.registerRule(std::make_shared<EnclosureRule>("metal1", "metal2", 5.0));
        m_engine.registerRule(std::make_shared<ContainmentRule>("metal1", "metal2"));
    }
    else
    {
        // if JSON didn't specify intersection/spacing rules, append sensible defaults
        if (needIntersection)
            m_engine.registerRule(std::make_shared<IntersectionRule>());
        if (needSpacing)
        {
            // default spacing values for layers 0 and 1; these could be refined later
            m_engine.registerRule(std::make_shared<MinSpacingRule>(10, 0));
            m_engine.registerRule(std::make_shared<MinSpacingRule>(5, 1));
        }
    }

    // Execute DRC engine
    DrcReport report = m_engine.run(*m_layout);

    // Save last report for selection interactions
    m_lastReport = report;

    // Update scene colors based on violations.
    m_scene->updateViolations(report);

    // Populate violation list for user reference (showing ruleName and shape IDs)
    m_violationList->clear();
    const auto& violations = report.getViolations();
    for (size_t i = 0; i < violations.size(); ++i)
    {
        const auto& violation = violations[i];
        // Format: [ruleName] ID:5, ID:7 (or just ID:5 if single shape)
        std::string idStr = "ID:" + std::to_string(violation.getShape1().getId());
        if (violation.getShape1().getId() != violation.getShape2().getId())
        {
            idStr += ", ID:" + std::to_string(violation.getShape2().getId());
        }
        QListWidgetItem* item = new QListWidgetItem(QString::fromStdString("[" + violation.getRuleName() + "] " + idStr));
        item->setData(Qt::UserRole, (int)i);
        m_violationList->addItem(item);
    }

    if (m_statusViolations)
        m_statusViolations->setText(QString("Violations: %1").arg((int)report.getViolationCount()));

    // Show summary
    QMessageBox::information(this, "DRC Complete",
                             QString::fromStdString(report.getSummary()));
}

void MainWindow::onViolationSelected()
{
    auto items = m_violationList->selectedItems();
    if (items.empty())
    {
        // Nothing selected; reset highlight
        if (m_scene)
            m_scene->resetHighlight();
        return;
    }

    // Collect the selected violation indices and corresponding shape IDs
    std::vector<int> shapeIds;
    for (auto* it : items)
    {
        bool ok = false;
        int idx = it->data(Qt::UserRole).toInt(&ok);
        if (!ok)
            continue;
        const auto& violations = m_lastReport.getViolations();
        if (idx < 0 || idx >= (int)violations.size())
            continue;
        const auto& v = violations[idx];
        shapeIds.push_back(v.getShape1().getId());
        shapeIds.push_back(v.getShape2().getId());
    }

    if (m_scene)
        m_scene->highlightShapeIds(shapeIds);

    // Zoom to bounding box of selected shapes
    QRectF zoomRect;
    bool first = true;
    for (int id : shapeIds)
    {
        // Try to find item and union its rect
        for (QGraphicsItem* gi : m_scene->items())
        {
            if (auto* r = qgraphicsitem_cast<QGraphicsRectItem*>(gi))
            {
                if (r->data(0).isValid() && r->data(0).toInt() == id)
                {
                    // Use sceneBoundingRect() to get coordinates in scene space
                    QRectF rrect = r->sceneBoundingRect();
                    if (first) { zoomRect = rrect; first = false; }
                    else zoomRect = zoomRect.united(rrect);
                }
            }
        }
    }

    if (!first && m_view)
    {
        m_view->fitInView(zoomRect, Qt::KeepAspectRatio);
    }
}

