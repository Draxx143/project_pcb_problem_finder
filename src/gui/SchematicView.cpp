// SchematicView.cpp — Industrial schematic sheet renderer implementation.
#include "SchematicView.h"
#include <QPainter>
#include <QWheelEvent>
#include <QMouseEvent>
#include <cmath>
#include <algorithm>
#include <map>

namespace {
inline QString qs(const std::string& s) {
    return QString::fromUtf8(s.data(), static_cast<int>(s.size()));
}
} // namespace

SchematicView::SchematicView(QWidget* parent) : QWidget(parent) {
    setMouseTracking(true);
    setMinimumSize(320, 240);
}

void SchematicView::setDocument(const pcbv::Document& doc) {
    document_ = doc;
    selectedIndex_ = -1;
    hoveredIndex_ = -1;
    rebuildLayout();
    fitToWindow();
    update();
}

void SchematicView::clear() {
    document_ = pcbv::Document();
    symbols_.clear();
    update();
}

bool SchematicView::hasSchematic() const {
    return document_.type == pcbv::DocType::SchDoc &&
           (!document_.components.empty() || !document_.geometry.tracks.empty());
}

void SchematicView::rebuildLayout() {
    symbols_.clear();
    if (document_.components.empty()) {
        sheetBounds_ = QRectF(0, 0, 1200, 800);
        return;
    }

    // Check if components already have distinct schematic coordinates
    bool hasValidCoords = false;
    double minX = 1e9, minY = 1e9, maxX = -1e9, maxY = -1e9;
    for (const auto& c : document_.components) {
        if (std::abs(c.x) >0.01 || std::abs(c.y) >0.01) hasValidCoords = true;
        minX = std::min(minX, c.x);
        minY = std::min(minY, c.y);
        maxX = std::max(maxX, c.x);
        maxY = std::max(maxY, c.y);
    }
    if ((maxX - minX) < 10.0 && (maxY - minY) < 10.0) hasValidCoords = false;

    const int n = static_cast<int>(document_.components.size());
    const int cols = std::max(2, static_cast<int>(std::ceil(std::sqrt(n))));
    const double cellW = 280.0;
    const double cellH = 170.0;
    const double boxW = 120.0;
    const double boxH = 64.0;

    minX = 1e9; minY = 1e9; maxX = -1e9; maxY = -1e9;

    for (int i = 0; i < n; ++i) {
        const auto& c = document_.components[i];
        double cx = hasValidCoords ? c.x : (160.0 + (i % cols) * cellW);
        double cy = hasValidCoords ? -c.y : (140.0 + (i / cols) * cellH);

        PlacedSymbol sym;
        sym.compIndex = i;
        sym.bodyRect = QRectF(cx - boxW * 0.5, cy - boxH * 0.5, boxW, boxH);
        sym.leftPin  = QPointF(sym.bodyRect.left() - 28.0, cy);
        sym.rightPin = QPointF(sym.bodyRect.right() + 28.0, cy);
        symbols_.push_back(sym);

        minX = std::min(minX, sym.leftPin.x() - 60.0);
        minY = std::min(minY, sym.bodyRect.top() - 60.0);
        maxX = std::max(maxX, sym.rightPin.x() + 60.0);
        maxY = std::max(maxY, sym.bodyRect.bottom() + 60.0);
    }

    sheetBounds_ = QRectF(QPointF(minX, minY), QPointF(maxX, maxY));
    if (sheetBounds_.width() < 600.0) sheetBounds_.setWidth(600.0);
    if (sheetBounds_.height() < 400.0) sheetBounds_.setHeight(400.0);
}

void SchematicView::fitToWindow() {
    if (sheetBounds_.isEmpty() || width() <= 10 || height() <= 10) return;
    const double sx = (width() - 80.0) / sheetBounds_.width();
    const double sy = (height() - 80.0) / sheetBounds_.height();
    zoom_ = std::max(0.1, std::min(sx, sy));
    const QPointF center = sheetBounds_.center();
    pan_ = QPointF(width() * 0.5 - center.x() * zoom_, height() * 0.5 - center.y() * zoom_);
    update();
}

QPointF SchematicView::sheetToWidget(const QPointF& pt) const {
    return QPointF(pt.x() * zoom_ + pan_.x(), pt.y() * zoom_ + pan_.y());
}

QPointF SchematicView::widgetToSheet(const QPointF& pt) const {
    return QPointF((pt.x() - pan_.x()) / zoom_, (pt.y() - pan_.y()) / zoom_);
}

void SchematicView::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    if (pan_.isNull()) fitToWindow();
}

void SchematicView::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // Industrial Dark Schematic Background
    painter.fillRect(rect(), QColor(15, 19, 24));

    if (symbols_.empty() && document_.geometry.tracks.empty()) {
        painter.setPen(QColor(138, 146, 158));
        painter.drawText(rect(), Qt::AlignCenter, tr("No schematic symbols in this document"));
        return;
    }

    painter.save();
    painter.translate(pan_);
    painter.scale(zoom_, zoom_);

    // 1. Sheet frame & title block
    const QRectF frame = sheetBounds_.adjusted(-30, -30, 30, 50);
    painter.setPen(QPen(QColor(58, 66, 78), 1.5));
    painter.setBrush(QColor(18, 23, 30));
    painter.drawRect(frame);

    // Subtle schematic grid
    painter.setPen(QPen(QColor(33, 39, 48), 1.0));
    const double step = 40.0;
    for (double x = std::floor(frame.left() / step) * step; x < frame.right(); x += step) {
        painter.drawLine(QPointF(x, frame.top()), QPointF(x, frame.bottom()));
    }
    for (double y = std::floor(frame.top() / step) * step; y < frame.bottom(); y += step) {
        painter.drawLine(QPointF(frame.left(), y), QPointF(frame.right(), y));
    }

    // 2. Explicit parsed wires (if present in geometry.tracks)
    painter.setPen(QPen(QColor(0, 210, 110, 210), 1.8));
    for (const auto& t : document_.geometry.tracks) {
        painter.drawLine(QPointF(t.x1, -t.y1), QPointF(t.x2, -t.y2));
    }

    // 3. Draw Orthogonal Schematic Interconnect Bus between components
    if (symbols_.size() >1 && document_.geometry.tracks.empty()) {
        painter.setPen(QPen(QColor(0, 190, 110, 170), 1.6));
        for (std::size_t i = 0; i + 1 < symbols_.size(); ++i) {
            const QPointF p1 = symbols_[i].rightPin;
            const QPointF p2 = symbols_[i + 1].leftPin;
            const double midX = (p1.x() + p2.x()) * 0.5;
            if (std::abs(p1.y() - p2.y()) < 5.0) {
                painter.drawLine(p1, p2);
            } else {
                const double busY = symbols_[i].bodyRect.bottom() + 34.0;
                painter.drawLine(p1, QPointF(p1.x() + 14.0, p1.y()));
                painter.drawLine(QPointF(p1.x() + 14.0, p1.y()), QPointF(p1.x() + 14.0, busY));
                painter.drawLine(QPointF(p1.x() + 14.0, busY), QPointF(p2.x() - 14.0, busY));
                painter.drawLine(QPointF(p2.x() - 14.0, busY), QPointF(p2.x() - 14.0, p2.y()));
                painter.drawLine(QPointF(p2.x() - 14.0, p2.y()), p2);
                Q_UNUSED(midX);
            }
        }
    }

    // 4. Draw Component Symbols, Pins, Designators, Values, and Footprints
    for (const auto& sym : symbols_) {
        const auto& c = document_.components[sym.compIndex];
        const bool isSelected = (sym.compIndex == selectedIndex_);
        const bool isHovered  = (sym.compIndex == hoveredIndex_);

        // Pin leads
        const int pinCount = std::max(2, !c.pins.empty() ? static_cast<int>(c.pins.size()) : 2);
        painter.setPen(QPen(QColor(180, 190, 205), 2.0));
        painter.drawLine(QPointF(sym.bodyRect.left(), sym.leftPin.y()), sym.leftPin);
        painter.drawLine(QPointF(sym.bodyRect.right(), sym.rightPin.y()), sym.rightPin);

        // Pin terminal circles
        painter.setBrush(QColor(0, 210, 110));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(sym.leftPin, 3.2, 3.2);
        painter.drawEllipse(sym.rightPin, 3.2, 3.2);

        if (pinCount >= 3) {
            QPointF bottomPin(sym.bodyRect.center().x(), sym.bodyRect.bottom() + 22.0);
            painter.setPen(QPen(QColor(180, 190, 205), 2.0));
            painter.drawLine(QPointF(sym.bodyRect.center().x(), sym.bodyRect.bottom()), bottomPin);
            painter.setBrush(QColor(0, 210, 110));
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(bottomPin, 3.2, 3.2);
        }

        // Pin numbers
        painter.setPen(QColor(138, 146, 158));
        painter.setFont(QFont(QStringLiteral("Segoe UI"), 7));
        painter.drawText(QRectF(sym.leftPin.x(), sym.leftPin.y() - 14, 24, 12), Qt::AlignCenter, QStringLiteral("1"));
        painter.drawText(QRectF(sym.rightPin.x() - 24, sym.rightPin.y() - 14, 24, 12), Qt::AlignCenter, QStringLiteral("2"));

        // Component symbol body
        QColor borderCol = isSelected ? QColor(0, 229, 255) : (isHovered ? QColor(255, 215, 64) : QColor(95, 145, 215));
        painter.setPen(QPen(borderCol, isSelected ? 2.4 : 1.6));
        painter.setBrush(QColor(24, 32, 44, 235));
        painter.drawRoundedRect(sym.bodyRect, 4.0, 4.0);

        // Designator (Top left above symbol)
        painter.setPen(QColor(255, 215, 64));
        painter.setFont(QFont(QStringLiteral("Segoe UI"), 9, QFont::Bold));
        painter.drawText(QRectF(sym.bodyRect.left(), sym.bodyRect.top() - 20, sym.bodyRect.width(), 18),
                         Qt::AlignLeft | Qt::AlignVCenter, qs(c.designator));

        // Value / Comment (Inside symbol center)
        QString valText = qs(c.comment.empty() ? c.designator : c.comment);
        if (valText.length() >16) valText = valText.left(15) + QStringLiteral("…");
        painter.setPen(QColor(230, 237, 243));
        painter.setFont(QFont(QStringLiteral("Segoe UI"), 8, QFont::DemiBold));
        painter.drawText(sym.bodyRect.adjusted(6, 4, -6, -18), Qt::AlignCenter, valText);

        // Footprint subtitle (Bottom inside symbol)
        QString fpText = qs(c.footprint);
        if (fpText.length() >20) fpText = fpText.left(19) + QStringLiteral("…");
        painter.setPen(QColor(139, 148, 158));
        painter.setFont(QFont(QStringLiteral("Segoe UI"), 7));
        painter.drawText(sym.bodyRect.adjusted(4, 26, -4, -4), Qt::AlignCenter, fpText);
    }

    // 5. Title Block in bottom-right of sheet
    QRectF titleBlock(frame.right() - 320, frame.bottom() - 42, 310, 34);
    painter.setPen(QPen(QColor(58, 66, 78), 1.2));
    painter.setBrush(QColor(22, 27, 34));
    painter.drawRect(titleBlock);
    painter.setPen(QColor(180, 190, 205));
    painter.setFont(QFont(QStringLiteral("Segoe UI"), 8, QFont::Bold));
    painter.drawText(titleBlock.adjusted(10, 0, -10, 0), Qt::AlignVCenter | Qt::AlignLeft,
                     QStringLiteral("SHEET: %1  |  PARTS: %2")
                         .arg(qs(document_.fileName))
                         .arg(document_.components.size()));

    painter.restore();
}

void SchematicView::wheelEvent(QWheelEvent* event) {
    const QPointF before = widgetToSheet(event->position());
    if (event->angleDelta().y() >0) zoom_ *= 1.18;
    else zoom_ /= 1.18;
    zoom_ = std::max(0.15, std::min(12.0, zoom_));
    const QPointF after = sheetToWidget(before);
    pan_ += (event->position() - after);
    update();
}

void SchematicView::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        const QPointF spt = widgetToSheet(event->pos());
        for (const auto& sym : symbols_) {
            if (sym.bodyRect.contains(spt)) {
                selectedIndex_ = sym.compIndex;
                emit componentClicked(sym.compIndex);
                update();
                return;
            }
        }
    }
    if (event->button() == Qt::MiddleButton || event->button() == Qt::LeftButton || event->button() == Qt::RightButton) {
        panning_ = true;
        panStart_ = event->pos();
        setCursor(Qt::ClosedHandCursor);
    }
}

void SchematicView::mouseMoveEvent(QMouseEvent* event) {
    if (panning_) {
        QPointF delta = event->pos() - panStart_;
        panStart_ = event->pos();
        pan_ += delta;
        update();
        return;
    }
    const QPointF spt = widgetToSheet(event->pos());
    int hit = -1;
    for (const auto& sym : symbols_) {
        if (sym.bodyRect.contains(spt)) { hit = sym.compIndex; break; }
    }
    if (hit != hoveredIndex_) {
        hoveredIndex_ = hit;
        update();
    }
}

void SchematicView::mouseReleaseEvent(QMouseEvent*) {
    panning_ = false;
    unsetCursor();
}

void SchematicView::mouseDoubleClickEvent(QMouseEvent*) {
    fitToWindow();
}
