#include "LayoutScene.hpp"
#include <QGraphicsRectItem>
#include <QBrush>
#include <QPen>
#include <QColor>
#include <QPainter>
#include <set>
#include <unordered_set>

#include <QVariant>
#include <cmath>

LayoutScene::LayoutScene(QObject* parent)
    : QGraphicsScene(parent)
{
    // Initialize a conservative layer->color mapping for common layer names
    m_layerColors.insert("metal1", QColor(70, 130, 255));   // blue
    m_layerColors.insert("metal2", QColor(60, 180, 75));    // green
    m_layerColors.insert("via", QColor(255, 215, 0));       // yellow
    m_layerColors.insert("diffusion", QColor(150, 50, 200)); // purple

    // Set scene background brush black so any uncovered area is dark
    setBackgroundBrush(QBrush(Qt::black));
}

void LayoutScene::loadLayout(const std::shared_ptr<Layout>& layout)
{
    // If layout is null, clear scene and return
    if (!layout)
    {
        clear();
        return;
    }

    // If same layout pointer as currently loaded, update existing items in-place
    if (m_layout && m_layout == layout)
    {
        // Keep track of shapes present this run
        std::unordered_set<int> seenIds;

        for (const auto& [layerId, layer] : layout->getLayers())
        {
            for (const auto& shape : layer.getShapes())
            {
                const Rect& bounds = shape.getBounds();
                QRectF sceneRect(bounds.min.x, bounds.min.y, bounds.width(), bounds.height());
                int sid = shape.getId();
                seenIds.insert(sid);

                auto it = m_shapeItems.find(sid);
                if (it != m_shapeItems.end() && it->second)
                {
                    // Update existing item geometry and reset style
                    it->second->setRect(sceneRect);
                    // store layer name for styling
                    it->second->setData(1, QString::fromStdString(layer.getName()));
                    it->second->setData(0, sid);
                    it->second->setZValue(static_cast<qreal>(layerId));
                    // Apply per-layer style
                    QString lname = it->second->data(1).toString();
                    QColor c = colorForLayerName(lname.toStdString());
                    QBrush b(c);
                    it->second->setBrush(b);
                    QPen pen(c.darker(140)); pen.setWidth(1);
                    it->second->setPen(pen);
                    it->second->setOpacity(1.0);
                }
                else
                {
                    // Create new item and store
                    QGraphicsRectItem* item = addRect(sceneRect);
                    // store layer name for styling
                    item->setData(1, QString::fromStdString(layer.getName()));
                    QColor c = colorForLayerName(QString::fromStdString(layer.getName()).toStdString());
                    // Slightly vary fill per-shape to reduce exact-overlap color masking
                    int var = sid % 10; // small variance
                    QColor fill = c;
                    int h, s, v; fill.getHsv(&h, &s, &v);
                    v = std::max(20, std::min(255, v - var));
                    fill.setHsv(h, s, v);
                    item->setBrush(QBrush(fill));
                    QPen pen(fill.darker(140)); pen.setWidth(1);
                    item->setPen(pen);
                    item->setAcceptHoverEvents(false);
                    item->setCacheMode(QGraphicsItem::NoCache);
                    item->setData(0, sid);
                    item->setZValue(static_cast<qreal>(layerId));
                    m_shapeItems[sid] = item;
                }
            }
        }

        // Remove items that are no longer present
        for (auto it = m_shapeItems.begin(); it != m_shapeItems.end(); )
        {
            if (seenIds.find(it->first) == seenIds.end())
            {
                auto* item = it->second;
                if (item)
                {
                    removeItem(item);
                    delete item;
                }
                it = m_shapeItems.erase(it);
            }
            else
            {
                ++it;
            }
        }

        // Keep same layout pointer
        m_layout = layout;
        return;
    }

    // Different layout: clear and rebuild scene
    clear();
    m_layout = layout;

    // Iterate through all layers and shapes, creating items
    for (const auto& [layerId, layer] : m_layout->getLayers())
    {
        for (const auto& shape : layer.getShapes())
        {
            const Rect& bounds = shape.getBounds();
            // Create rectangle using exact layout coordinates (x=min.x, y=min.y, width, height)
            QRectF sceneRect(bounds.min.x, bounds.min.y, bounds.width(), bounds.height());

            // Create graphics item
            QGraphicsRectItem* item = addRect(sceneRect);

            // store layer name for styling
            item->setData(1, QString::fromStdString(layer.getName()));
            QColor c = colorForLayerName(layer.getName());
            // Vary fill slightly by shape id to avoid perfectly identical overlapping fills
            int var = shape.getId() % 12;
            QColor fill = c;
            int h, s, v; fill.getHsv(&h, &s, &v);
            v = std::max(18, std::min(255, v - var));
            fill.setHsv(h, s, v);
            item->setBrush(QBrush(fill));
            QPen pen(fill.darker(140)); pen.setWidth(1);
            item->setPen(pen);

            // Enable antialiasing for smoother rendering
            item->setAcceptHoverEvents(false);
            item->setCacheMode(QGraphicsItem::NoCache);
            item->setZValue(static_cast<qreal>(layerId));

            // Store reference for later updates (keyed by shapeId)
            // Tag the graphics item with the shape id for selection/lookup
            item->setData(0, shape.getId());
            m_shapeItems[shape.getId()] = item;
        }
    }
}

void LayoutScene::updateViolations(const DrcReport& report)
{
    // Collect all unique violating shape IDs from the report
    // For single-shape violations (e.g., MinWidth), shape1 == shape2, so only add once
    std::vector<int> ids;
    ids.reserve(report.getViolations().size() * 2);
    for (const auto& violation : report.getViolations())
    {
        ids.push_back(violation.getShape1().getId());
        // Only add shape2 if it's different from shape1
        if (violation.getShape1().getId() != violation.getShape2().getId())
        {
            ids.push_back(violation.getShape2().getId());
        }
    }

    // Highlight those shape IDs
    highlightShapeIds(ids);
}

void LayoutScene::highlightShapeIds(const std::vector<int>& shapeIds)
{
    // Build a lookup set for fast membership test
    std::unordered_map<int, bool> hit;
    for (int id : shapeIds)
        hit[id] = true;

    // Update all items: emphasize hits, dim others
    for (auto& kv : m_shapeItems)
    {
        QGraphicsRectItem* item = kv.second;
        if (!item)
            continue;

        if (hit.find(kv.first) != hit.end())
        {
            // Emphasize: add semi-transparent red overlay (so base color remains visible)
            item->setOpacity(1.0);
            if (!m_violationOverlays.contains(kv.first))
            {
                QGraphicsRectItem* overlay = addRect(item->rect());
                overlay->setBrush(QBrush(QColor(200, 40, 40, 90)));
                QPen pen(QColor(200, 30, 30)); pen.setWidth(2);
                overlay->setPen(pen);
                overlay->setZValue(item->zValue() + 0.5);
                overlay->setFlag(QGraphicsItem::ItemIsSelectable, false);
                overlay->setData(0, kv.first);
                m_violationOverlays.insert(kv.first, overlay);
            }
        }
        else
        {
            // Dim non-involved shapes: reduce opacity but keep base color
            item->setOpacity(0.28);
            // remove overlay if it exists
            if (m_violationOverlays.contains(kv.first))
            {
                auto* ov = m_violationOverlays.take(kv.first);
                if (ov) { removeItem(ov); delete ov; }
            }
        }
    }
}

void LayoutScene::resetHighlight()
{
    // Remove any violation overlays
    for (auto it = m_violationOverlays.begin(); it != m_violationOverlays.end(); ++it)
    {
        QGraphicsRectItem* ov = it.value();
        if (ov) { removeItem(ov); delete ov; }
    }
    m_violationOverlays.clear();

    // Reset opacity and restore per-layer pen/brush for all shapes
    for (auto& kv : m_shapeItems)
    {
        QGraphicsRectItem* item = kv.second;
        if (!item) continue;
        item->setOpacity(1.0);
        QString lname = item->data(1).toString();
        QColor c = colorForLayerName(lname.toStdString());
        // try to retain the small per-shape variation
        int sid = kv.first;
        int var = sid % 12;
        QColor fill = c; int h,s,v; fill.getHsv(&h,&s,&v); v = std::max(18, std::min(255, v - var)); fill.setHsv(h,s,v);
        item->setBrush(QBrush(fill));
        QPen pen(fill.darker(140)); pen.setWidth(1); item->setPen(pen);
    }
}

void LayoutScene::clear()
{
    QGraphicsScene::clear();
    m_shapeItems.clear();
    m_layout = nullptr;
    // clear overlays
    for (auto it = m_violationOverlays.begin(); it != m_violationOverlays.end(); ++it)
    {
        QGraphicsRectItem* ov = it.value();
        if (ov) delete ov;
    }
    m_violationOverlays.clear();
}

QRectF LayoutScene::getLayoutBounds() const
{
    return itemsBoundingRect();
}

void LayoutScene::applyNormalStyle(QGraphicsRectItem* item)
{
    if (!item)
        return;
    // If a layer name exists, use its color, otherwise fallback
    QString lname = item->data(1).toString();
    QColor c = colorForLayerName(lname.toStdString());
    item->setBrush(QBrush(c));
    QPen pen(c.darker(140)); pen.setWidth(1);
    pen.setCapStyle(Qt::SquareCap);
    pen.setJoinStyle(Qt::MiterJoin);
    item->setPen(pen);
    item->setOpacity(1.0);
}

void LayoutScene::applyViolationStyle(QGraphicsRectItem* item)
{
    if (!item)
        return;
    // Keep base coloring and rely on overlay; this function left for backward compatibility
    QPen pen(QColor(180, 20, 20));
    pen.setWidth(2);
    item->setPen(pen);
    item->setOpacity(1.0);
}

QColor LayoutScene::colorForLayerName(const std::string& name) const
{
    QString key = QString::fromStdString(name).toLower();
    if (m_layerColors.contains(key))
        return m_layerColors.value(key);

    // Fallback: deterministically generate a pleasing color from the name hash
    uint32_t h = qHash(key);
    int hue = (int)(h % 360);
    QColor c;
    c.setHsv(hue, 200, 220);
    return c;
}

void LayoutScene::drawBackground(QPainter* painter, const QRectF& rect)
{
    // Fill background black
    painter->save();
    painter->fillRect(rect, Qt::black);

    // Draw faint dot grid in scene coordinates
    painter->setPen(Qt::NoPen);
    painter->setBrush(m_gridDotColor);

    const int spacing = m_gridSpacing;
    // determine start positions aligned to spacing
    int xstart = static_cast<int>(std::floor(rect.left() / spacing) * spacing);
    int ystart = static_cast<int>(std::floor(rect.top() / spacing) * spacing);

    const qreal dotSize = 1.2; // scene units
    for (int x = xstart; x <= rect.right(); x += spacing)
    {
        for (int y = ystart; y <= rect.bottom(); y += spacing)
        {
            painter->drawEllipse(QPointF(x + 0.0, y + 0.0), dotSize, dotSize);
        }
    }

    painter->restore();
}
