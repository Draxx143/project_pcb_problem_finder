#pragma once
#include "../simulator/SpiceSimulator.h"
#include <cmath>
#include <memory>

namespace CircuitGuard {

struct CompilationDiagnostic {
    enum class Severity { Info, Warning, Error, Critical };
    Severity severity;
    QString ruleId;
    QString targetObject;
    double expectedValue = 0.0;
    double simulatedValue = 0.0;
    QString rawMessage;
    QString aiRootCauseExplanation;
};

class ICompilerRule {
public:
    virtual ~ICompilerRule() = default;
    virtual QVector<CompilationDiagnostic> evaluate(const Netlist& netlist, const SimulationResult& sim) const = 0;
};

class ComponentSOARule : public ICompilerRule {
public:
    QVector<CompilationDiagnostic> evaluate(const Netlist& netlist, const SimulationResult& sim) const override {
        QVector<CompilationDiagnostic> diags;
        for (const auto& comp : netlist.components) {
            double pSim = std::abs(sim.componentPowers.value(comp.refDes, 0.0));
            if (comp.ratings.maxPower.has_value() && pSim > comp.ratings.maxPower.value()) {
                diags.append({
                    CompilationDiagnostic::Severity::Critical,
                    "SOA_OVERPOWER",
                    comp.refDes,
                    comp.ratings.maxPower.value(),
                    pSim,
                    QString("قطعه %1 توان %2W مصرف می‌کند که بیش از حد مجاز (%3W) است.")
                        .arg(comp.refDes).arg(pSim, 0, 'f', 3).arg(comp.ratings.maxPower.value()),
                    ""
                });
            }
        }
        return diags;
    }
};

class NetExpectedConstraintRule : public ICompilerRule {
public:
    QVector<CompilationDiagnostic> evaluate(const Netlist& netlist, const SimulationResult& sim) const override {
        QVector<CompilationDiagnostic> diags;
        for (const auto& net : netlist.nets) {
            if (!net.constraint.has_value()) continue;
            const auto& c = net.constraint.value();

            QString sourceKey = "V_" + net.name.toUpper();
            if (c.expectedCurrentMin.has_value() && sim.branchCurrents.contains(sourceKey)) {
                double iSim = std::abs(sim.branchCurrents.value(sourceKey));
                if (iSim < c.expectedCurrentMin.value()) {
                    diags.append({
                        CompilationDiagnostic::Severity::Error,
                        "ERC_NET_UNDERCURRENT",
                        net.name,
                        c.expectedCurrentMin.value(),
                        iSim,
                        QString("در نت %1، جریان مورد انتظار %2mA است اما شبیه‌سازی %3mA نشان می‌دهد.")
                            .arg(net.name)
                            .arg(c.expectedCurrentMin.value() * 1000.0, 0, 'f', 1)
                            .arg(iSim * 1000.0, 0, 'f', 1),
                        ""
                    });
                }
            }
        }
        return diags;
    }
};

class CircuitCompiler {
public:
    CircuitCompiler() {
        rules_.append(std::make_shared<ComponentSOARule>());
        rules_.append(std::make_shared<NetExpectedConstraintRule>());
    }

    QVector<CompilationDiagnostic> compile(const Netlist& netlist, const QMap<QString, double>& powerRails, SimulationResult& outSim) {
        SpiceSimulator sim;
        outSim = sim.runOperatingPoint(netlist, powerRails);
        QVector<CompilationDiagnostic> allDiagnostics;
        if (!outSim.success) {
            allDiagnostics.append({
                CompilationDiagnostic::Severity::Critical,
                "SIM_ENGINE_FAILURE",
                netlist.designName,
                0, 0,
                outSim.errorMessage,
                ""
            });
            return allDiagnostics;
        }
        for (const auto& rule : rules_) {
            allDiagnostics.append(rule->evaluate(netlist, outSim));
        }
        return allDiagnostics;
    }

private:
    QVector<std::shared_ptr<ICompilerRule>> rules_;
};

} // namespace CircuitGuard
