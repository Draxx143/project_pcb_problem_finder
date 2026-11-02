#pragma once
#include <QString>
#include <QVector>
#include <QMap>
#include <QVariant>
#include <optional>

namespace CircuitGuard {

struct ElectricalRatings {
    std::optional<double> maxVoltage;
    std::optional<double> maxCurrent;
    std::optional<double> maxPower;
    std::optional<double> minFreq;
    std::optional<double> maxFreq;
};

struct SpiceModel {
    enum class Type { Resistor, Capacitor, Inductor, Diode, BJT, MOSFET, Subcircuit, VoltageSource, Unknown };
    Type type = Type::Unknown;
    QString modelName;
    QString inlineDefinition;
    QString externalLibPath;
    QMap<QString, int> pinMapping;
};

struct ExpectedConstraint {
    QString targetNetOrPin;
    std::optional<double> nominalVoltage;
    double voltageTolerancePct = 5.0;
    std::optional<double> expectedCurrentMin;
    std::optional<double> expectedCurrentMax;
};

struct ComponentInstance {
    QString refDes;
    QString value;
    QString footprint;
    SpiceModel spiceModel;
    ElectricalRatings ratings;
    QMap<QString, QString> pinToNetMap;
};

struct PinRef {
    QString componentRef;
    QString pinNumber;
};

struct Net {
    QString name;
    QVector<PinRef> connectedPins;
    std::optional<ExpectedConstraint> constraint;
};

class Netlist {
public:
    QString designName = "Untitled_Circuit";
    QMap<QString, ComponentInstance> components;
    QMap<QString, Net> nets;

    QVector<QString> getAdjacentComponents(const QString& netName) const {
        QVector<QString> result;
        if (!nets.contains(netName)) return result;
        for (const auto& pin : nets[netName].connectedPins) {
            if (!result.contains(pin.componentRef))
                result.append(pin.componentRef);
        }
        return result;
    }
};

class INetlistProvider {
public:
    virtual ~INetlistProvider() = default;
    virtual bool loadFromFile(const QString& filePath, Netlist& outNetlist, QString& errorMsg) = 0;
};

} // namespace CircuitGuard
