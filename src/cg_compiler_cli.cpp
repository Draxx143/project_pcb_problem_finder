#include <QCoreApplication>
#include <QTextStream>
#include "compiler/CircuitCompiler.h"
#include "ai/AiDiagnosticExplainer.h"

using namespace CircuitGuard;

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    QTextStream out(stdout);

    Netlist netlist;
    netlist.designName = "Power_Rail_Test_Board";

    ComponentInstance r12;
    r12.refDes = "R12";
    r12.value = "66";
    r12.spiceModel.type = SpiceModel::Type::Resistor;
    r12.ratings.maxPower = 0.25;
    r12.pinToNetMap["1"] = "VCC_3V3";
    r12.pinToNetMap["2"] = "GND";
    netlist.components["R12"] = r12;

    Net vccNet;
    vccNet.name = "VCC_3V3";
    vccNet.connectedPins.append({"R12", "1"});
    vccNet.constraint = ExpectedConstraint{"VCC_3V3", 3.3, 5.0, 0.300, 0.500};
    netlist.nets["VCC_3V3"] = vccNet;

    Net gndNet;
    gndNet.name = "GND";
    gndNet.connectedPins.append({"R12", "2"});
    netlist.nets["GND"] = gndNet;

    CircuitCompiler compiler;
    SimulationResult simResult;
    auto diagnostics = compiler.compile(netlist, {{"VCC_3V3", 3.3}}, simResult);

    out << "\n=== CircuitGuard Pro Compiler Report ===\n";
    for (auto& diag : diagnostics) {
        diag.aiRootCauseExplanation = AiDiagnosticExplainer::explainDiagnostic(diag, netlist, simResult);
        out << "[ERROR] " << diag.rawMessage << "\n";
        out << "   -> " << diag.aiRootCauseExplanation << "\n";
    }
    return 0;
}
