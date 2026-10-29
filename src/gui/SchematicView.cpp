// SchematicView.cpp — Implementation of schematic graphical rendering.
#include "SchematicView.h"
#include <QPainter>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QWheelEvent>
#include <QMouseEvent>
#include <cmath>

inline QString qs(const std::string& s) { return QString::fromUtf8(s.data(), static_cast<int>(s.size())); }



SchematicView::SchematicView(QWidget* parent) : QWidget(parent) {
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
}

void SchematicView::setDocument(const pcbv::Document& doc) {
    document_ = doc;
    zoom_ = 1.0;
    pan_ = QPointF(0, 0);
    update();
}

void SchematicView::clear() {
    document_ = pcbv::Document();
    update();
}

void SchematicView::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // Dark professional background
    painter.fillRect(rect(), QColor(13, 17, 23));

    painter.save();
    painter.translate(width() / 2.0 + pan_.x(), height() / 2.0 + pan_.y());
    painter.scale(zoom_, zoom_);

    // Draw grid dots
    painter.setPen(QColor(48, 54, 61));
    for (double x = -2000; x <= 2000; x += 100) {
        for (double y = -2000; y <= 2000; y += 100) {
            painter.drawPoint(QPointF(x, y));
        }
    }

    // Draw schematic wires/tracks
    painter.setPen(QPen(QColor(0, 229, 255, 200), 1.5));
    for (const auto& t : document_.geometry.tracks) {
        painter.drawLine(QPointF(t.x1, t.y1), QPointF(t.x2, t.y2));
    }

    // Draw schematic components (symbol boxes & designators)
    for (const auto& c : document_.components) {
        QRectF box(c.x - 30, c.y - 20, 60, 40);
        painter.setPen(QPen(QColor(255, 255, 255, 220), 1.2));
        painter.setBrush(QColor(22, 27, 34, 200));
        painter.drawRoundedRect(box, 4, 4);

        painter.setPen(QColor(255, 215, 64));
        painter.setFont(QFont("Segoe UI", 9, QFont::Bold));
        painter.drawText(box, Qt::AlignCenter, qs(c.designator));
    }

    painter.restore();
}

void SchematicView::wheelEvent(QWheelEvent* event) {
    if (event->angleDelta().y() > 0) zoom_ *= 1.15;
    else zoom_ /= 1.15;
    if (zoom_ < 0.1) zoom_ = 0.1;
    if (zoom_ > 20.0) zoom_ = 20.0;
    update();
}

void SchematicView::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::MiddleButton || event->button() == Qt::LeftButton) {
        panning_ = true;
        panStart_ = event->pos();
    }
}

void SchematicView::mouseMoveEvent(QMouseEvent* event) {
    if (panning_) {
        QPointF delta = event->pos() - panStart_;
        panStart_ = event->pos();
        pan_ += delta;
        update();
    }
}
