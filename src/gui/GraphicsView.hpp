#pragma once

#include <QGraphicsView>
#include <QWheelEvent>
#include <QMouseEvent>

// Simple QGraphicsView subclass that enables zooming with mouse wheel
// and panning with middle mouse button or left-button drag with Alt.
class GraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit GraphicsView(QWidget* parent = nullptr);

protected:
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    bool m_panning = false;
    QPoint m_lastPanPoint;
};
