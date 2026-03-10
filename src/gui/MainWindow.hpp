#pragma once

#include <QMainWindow>
#include <QListWidget>
#include <QLabel>
#include <QToolBar>
#include <memory>

#include "GraphicsView.hpp"
#include "LayoutScene.hpp"

#include <io/JsonLayoutLoader.hpp>
#include <core/DrcEngine.hpp>
#include <drc/DrcReport.hpp>

// MainWindow provides minimal UI to load a JSON layout, run DRC, and visualize results.
// Interaction with DRC core is done through headers only; no Qt types leak into core.
//
// The GUI separates rendering concerns:
// - LayoutScene: Manages shape rendering and violation highlighting
// - MainWindow: Coordinates user actions and DRC execution
// - GraphicsView: Provides zoom/pan interactions
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

    /// Load a layout from a file path (used for command-line launching)
    void loadLayoutFromPath(const QString& path);

private slots:
    void onOpenLayout();
    void onRunDrc();
    void onViolationSelected();

private:
    void buildMenu();

    GraphicsView* m_view = nullptr;
    LayoutScene* m_scene = nullptr;
    QListWidget* m_violationList = nullptr;

    QToolBar* m_toolbar = nullptr;
    QLabel* m_statusShapes = nullptr;
    QLabel* m_statusViolations = nullptr;

    std::shared_ptr<Layout> m_layout;
    DrcEngine m_engine;

    // Rules parsed from the loaded JSON (empty when using demo defaults)
    std::vector<DrcRulePtr> m_parsedRules;

    // Keep last report so selection interactions can access violations
    DrcReport m_lastReport;
};
