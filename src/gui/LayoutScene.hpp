#pragma once

#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <unordered_map>
#include <memory>
#include <QColor>
#include <QMap>
#include <QHash>

class QPainter;

#include <layout/Layout.hpp>
#include <drc/DrcReport.hpp>

// Scene for rendering layout shapes and violations
class LayoutScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit LayoutScene(QObject* parent = nullptr);

    void loadLayout(const std::shared_ptr<Layout>& layout);

    void updateViolations(const DrcReport& report);

    void clear();

    QRectF getLayoutBounds() const;

    void highlightShapeIds(const std::vector<int>& shapeIds);

    void resetHighlight();

private:
    // Mapping from shapeId to graphics item
    std::unordered_map<int, QGraphicsRectItem*> m_shapeItems;

    // Currently loaded layout
    std::shared_ptr<Layout> m_layout;

    /// @brief Apply normal style to a shape (light gray, black pen).
    void applyNormalStyle(QGraphicsRectItem* item);

    /// @brief Apply violation style to a shape (red, red pen with thickness).
    void applyViolationStyle(QGraphicsRectItem* item);

    // Draw custom background (black) with dotted grid
    void drawBackground(QPainter* painter, const QRectF& rect) override;

    // Resolve color for a given layer name (cached)
    QColor colorForLayerName(const std::string& name) const;

    // Overlay items used to highlight violations without replacing base color
    QHash<int, QGraphicsRectItem*> m_violationOverlays;

    // Map layer name -> color
    QMap<QString, QColor> m_layerColors;

    // Grid appearance
    QColor m_gridDotColor = QColor(80, 80, 80);
    int m_gridSpacing = 50; // layout units between dots

    
};
