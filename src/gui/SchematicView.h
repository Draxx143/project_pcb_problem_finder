// SchematicView.h — Industrial schematic sheet renderer widget.
#pragma once

#include <QWidget>
#include <QPointF>
#include <QRectF>
#include <vector>
#include "core/Model.h"

class SchematicView : public QWidget {
    Q_OBJECT
public:
    explicit SchematicView(QWidget* parent = nullptr);
    void setDocument(const pcbv::Document& doc);
    void clear();
    bool hasSchematic() const;
    void fitToWindow();

signals:
    void componentClicked(int index);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;

private:
    struct PlacedSymbol {
        int compIndex = -1;
        QRectF bodyRect;
        QPointF leftPin;
        QPointF rightPin;
    };

    void rebuildLayout();
    QPointF sheetToWidget(const QPointF& pt) const;
    QPointF widgetToSheet(const QPointF& pt) const;

    pcbv::Document document_;
    std::vector<PlacedSymbol>symbols_;
    QRectF sheetBounds_;
    double zoom_ = 1.0;
    QPointF pan_;
    bool panning_ = false;
    QPointF panStart_;
    int hoveredIndex_ = -1;
    int selectedIndex_ = -1;
};
