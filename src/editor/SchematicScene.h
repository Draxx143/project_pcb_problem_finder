#pragma once
#include "../core/CoreDataStructures.h"
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QPen>

namespace CircuitGuard {

class SchematicPinItem : public QGraphicsEllipseItem {
public:
    QString componentRef;
    QString pinNumber;
    QString assignedNet;

    SchematicPinItem(const QString& ref, const QString& pin, QGraphicsItem* parent = nullptr)
        : QGraphicsEllipseItem(-4, -4, 8, 8, parent), componentRef(ref), pinNumber(pin) {
        setBrush(Qt::red);
    }
};

class SchematicSymbolItem : public QGraphicsRectItem {
public:
    ComponentInstance componentData;
    QVector<SchematicPinItem*> pins;

    SchematicSymbolItem(const ComponentInstance& comp)
        : QGraphicsRectItem(0, 0, 60, 30), componentData(comp) {
        setFlag(QGraphicsItem::ItemIsMovable);
        setFlag(QGraphicsItem::ItemIsSelectable);
        setPen(QPen(Qt::darkBlue, 2));

        auto* pin1 = new SchematicPinItem(comp.refDes, "1", this);
        pin1->setPos(0, 15);
        auto* pin2 = new SchematicPinItem(comp.refDes, "2", this);
        pin2->setPos(60, 15);
        pins << pin1 << pin2;
    }
};

class SchematicScene : public QGraphicsScene {
    Q_OBJECT
public:
    explicit SchematicScene(QObject* parent = nullptr) : QGraphicsScene(parent) {}

    void addSymbol(const ComponentInstance& comp, const QPointF& pos) {
        auto* item = new SchematicSymbolItem(comp);
        item->setPos(pos);
        addItem(item);
        symbols_.append(item);
    }

    void connectPinsWithWire(SchematicPinItem* p1, SchematicPinItem* p2, const QString& netName) {
        p1->assignedNet = netName;
        p2->assignedNet = netName;
        addLine(QLineF(p1->scenePos(), p2->scenePos()), QPen(Qt::darkGreen, 2));
    }

    Netlist extractNetlistFromCanvas() const {
        Netlist nl;
        nl.designName = "Interactive_Schematic";
        for (auto* sym : symbols_) {
            ComponentInstance comp = sym->componentData;
            for (auto* pin : sym->pins) {
                QString netName = pin->assignedNet.isEmpty() ? "NC_" + comp.refDes + "_" + pin->pinNumber : pin->assignedNet;
                comp.pinToNetMap[pin->pinNumber] = netName;
                nl.nets[netName].name = netName;
                nl.nets[netName].connectedPins.append({comp.refDes, pin->pinNumber});
            }
            nl.components[comp.refDes] = comp;
        }
        return nl;
    }

private:
    QVector<SchematicSymbolItem*> symbols_;
};

} // namespace CircuitGuard
