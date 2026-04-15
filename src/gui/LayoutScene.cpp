#include "LayoutScene.hpp"
#include <QGraphicsRectItem>
#include <QGraphicsPolygonItem>
#include <QBrush>
#include <QPen>
#include <QColor>
#include <QPainter>
#include <set>
#include <unordered_set>

#include <QVariant>
#include <cmath>

static QPolygonF pointsToPolygon(const std::vector<Point>& points)
{
    QPolygonF polygon;
    polygon.reserve(static_cast<int>(points.size()));
    for (const auto& point : points)
    {
        polygon << QPointF(point.x, point.y);
    }
    return polygon;
}

static QAbstractGraphicsShapeItem* asShapeItem(QGraphicsItem* item)
{
    return dynamic_cast<QAbstractGraphicsShapeItem*>(item);
}

static bool itemMatchesShapeType(QGraphicsItem* item, const Shape& shape)
{
    if (shape.hasPoints())
        return dynamic_cast<QGraphicsPolygonItem*>(item) != nullptr;
    return dynamic_cast<QGraphicsRectItem*>(item) != nullptr;
}

static QGraphicsItem* createShapeGraphicsItem(LayoutScene* scene,
                                              const Shape& shape,
                                              const Layer& layer,
                                              const QColor& baseColor)
{
    QGraphicsItem* item = nullptr;
    if (shape.hasPoints())
    {
        auto* polygonItem = scene->addPolygon(pointsToPolygon(shape.getPoints()));
        item = polygonItem;
    }
    else
    {
        const Rect& bounds = shape.getBounds();
        QRectF sceneRect(bounds.min.x, bounds.min.y, bounds.width(), bounds.height());
        auto* rectItem = scene->addRect(sceneRect);
        item = rectItem;
    }

    if (auto* shapeItem = asShapeItem(item))
    {
        int var = shape.getId() % 12;
        QColor fill = baseColor;
        int h, s, v; fill.getHsv(&h, &s, &v);
        v = std::max(18, std::min(255, v - var));
        fill.setHsv(h, s, v);
        shapeItem->setBrush(QBrush(fill));
        QPen pen(fill.darker(140)); pen.setWidth(1);
        shapeItem->setPen(pen);
        shapeItem->setOpacity(1.0);
    }

    item->setData(1, QString::fromStdString(layer.getName()));
    item->setData(0, shape.getId());
    item->setZValue(static_cast<qreal>(layer.getId()));
    return item;
}

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
                int sid = shape.getId();
                seenIds.insert(sid);

                auto it = m_shapeItems.find(sid);
                if (it != m_shapeItems.end() && it->second && itemMatchesShapeType(it->second, shape))
                {
                    QGraphicsItem* item = it->second;
                    if (shape.hasPoints())
                    {
                        auto* polygonItem = dynamic_cast<QGraphicsPolygonItem*>(item);
                        if (polygonItem)
                            polygonItem->setPolygon(pointsToPolygon(shape.getPoints()));
                    }
                    else
                    {
                        auto* rectItem = dynamic_cast<QGraphicsRectItem*>(item);
                        if (rectItem)
                        {
                            const Rect& bounds = shape.getBounds();
                            QRectF sceneRect(bounds.min.x, bounds.min.y, bounds.width(), bounds.height());
                            rectItem->setRect(sceneRect);
                        }
                    }

                    item->setData(1, QString::fromStdString(layer.getName()));
                    item->setData(0, sid);
                    item->setZValue(static_cast<qreal>(layerId));
                    if (auto* shapeItem = asShapeItem(item))
                    {
                        QString lname = shapeItem->data(1).toString();
                        QColor c = colorForLayerName(lname.toStdString());
                        QBrush b(c);
                        shapeItem->setBrush(b);
                        QPen pen(c.darker(140)); pen.setWidth(1);
                        shapeItem->setPen(pen);
                        shapeItem->setOpacity(1.0);
                    }
                }
                else
                {
                    if (it != m_shapeItems.end() && it->second)
                    {
                        removeItem(it->second);
                        delete it->second;
                        m_shapeItems.erase(it);
                    }

                    QColor c = colorForLayerName(layer.getName());
                    QGraphicsItem* item = createShapeGraphicsItem(this, shape, layer, c);
                    if (item)
                    {
                        item->setAcceptHoverEvents(false);
                        item->setCacheMode(QGraphicsItem::NoCache);
                        m_shapeItems[sid] = item;
                    }
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
            QColor c = colorForLayerName(layer.getName());
            QGraphicsItem* item = createShapeGraphicsItem(this, shape, layer, c);
            if (!item)
                continue;

            item->setAcceptHoverEvents(false);
            item->setCacheMode(QGraphicsItem::NoCache);
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
        QGraphicsItem* item = kv.second;
        if (!item)
            continue;

        if (hit.find(kv.first) != hit.end())
        {
            item->setOpacity(1.0);
            if (!m_violationOverlays.contains(kv.first))
            {
                if (auto* polygonItem = dynamic_cast<QGraphicsPolygonItem*>(item))
                {
                    QGraphicsPolygonItem* overlay = addPolygon(polygonItem->polygon());
                    overlay->setBrush(QBrush(QColor(200, 40, 40, 90)));
                    QPen pen(QColor(200, 30, 30)); pen.setWidth(2);
                    overlay->setPen(pen);
                    overlay->setZValue(item->zValue() + 0.5);
                    overlay->setFlag(QGraphicsItem::ItemIsSelectable, false);
                    overlay->setData(0, kv.first);
                    m_violationOverlays.insert(kv.first, overlay);
                }
                else if (auto* rectItem = dynamic_cast<QGraphicsRectItem*>(item))
                {
                    QGraphicsRectItem* overlay = addRect(rectItem->rect());
                    overlay->setBrush(QBrush(QColor(200, 40, 40, 90)));
                    QPen pen(QColor(200, 30, 30)); pen.setWidth(2);
                    overlay->setPen(pen);
                    overlay->setZValue(item->zValue() + 0.5);
                    overlay->setFlag(QGraphicsItem::ItemIsSelectable, false);
                    overlay->setData(0, kv.first);
                    m_violationOverlays.insert(kv.first, overlay);
                }
            }
        }
        else
        {
            item->setOpacity(0.28);
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
        if (auto* ov = it.value())
        {
            removeItem(ov);
            delete ov;
        }
    }
    m_violationOverlays.clear();

    // Reset opacity and restore per-layer pen/brush for all shapes
    for (auto& kv : m_shapeItems)
    {
        QGraphicsItem* item = kv.second;
        if (!item)
            continue;

        item->setOpacity(1.0);
        if (auto* shapeItem = asShapeItem(item))
        {
            QString lname = item->data(1).toString();
            QColor c = colorForLayerName(lname.toStdString());
            int sid = kv.first;
            int var = sid % 12;
            QColor fill = c;
            int h, s, v;
            fill.getHsv(&h, &s, &v);
            v = std::max(18, std::min(255, v - var));
            fill.setHsv(h, s, v);
            shapeItem->setBrush(QBrush(fill));
            QPen pen(fill.darker(140)); pen.setWidth(1);
            shapeItem->setPen(pen);
        }
    }
}

void LayoutScene::clear()
{
    QGraphicsScene::clear();
    m_shapeItems.clear();
    m_layout = nullptr;
    m_violationOverlays.clear();
}

QRectF LayoutScene::getLayoutBounds() const
{
    return itemsBoundingRect();
}

void LayoutScene::applyNormalStyle(QAbstractGraphicsShapeItem* item)
{
    if (!item)
        return;
    QString lname = item->data(1).toString();
    QColor c = colorForLayerName(lname.toStdString());
    item->setBrush(QBrush(c));
    QPen pen(c.darker(140)); pen.setWidth(1);
    pen.setCapStyle(Qt::SquareCap);
    pen.setJoinStyle(Qt::MiterJoin);
    item->setPen(pen);
    item->setOpacity(1.0);
}

void LayoutScene::applyViolationStyle(QAbstractGraphicsShapeItem* item)
{
    if (!item)
        return;
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
