// SchematicView.h — Graphical schematic renderer widget.
#pragma once

#include <QWidget>
#include <QPointF>
#include "core/Model.h"

class SchematicView : public QWidget {
    Q_OBJECT
public:
    explicit SchematicView(QWidget* parent = nullptr);
    void setDocument(const pcbv::Document& doc);
    void clear();
    bool hasGeometry() const { return !document_.components.empty() || !document_.geometry.tracks.empty(); }

protected:
    void paintEvent(QPaintEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

private:
    pcbv::Document document_;
    double zoom_ = 1.0;
    QPointF pan_;
    bool panning_ = false;
    QPointF panStart_;
};
