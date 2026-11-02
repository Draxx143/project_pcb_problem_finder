#pragma once
#include "../core/CoreDataStructures.h"
#include <ngspice/sharedspice.h>
#include <cstdlib>
#include <cstring>

namespace CircuitGuard {

struct SimulationResult {
    bool success = false;
    QString errorMessage;
    QMap<QString, double> nodeVoltages;
    QMap<QString, double> branchCurrents;
    QMap<QString, double> componentPowers;
};

class SpiceSimulator {
public:
    SpiceSimulator() {
        ngSpice_Init(&SpiceSimulator::ngSendChar, &SpiceSimulator::ngSendStat,
                     &SpiceSimulator::ngControlledExit, nullptr, nullptr,
                     &SpiceSimulator::ngBGThreadRunning, this);
    }

    QString generateSpiceDeck(const Netlist& netlist, const QMap<QString, double>& sourceValues = {}) const {
        QStringList deck;
        deck << QString("* CircuitGuard Auto-Generated SPICE Deck: %1").arg(netlist.designName);

        for (const auto& comp : netlist.components) {
            if (comp.spiceModel.type == SpiceModel::Type::Resistor) {
                QString n1 = normalizeNode(comp.pinToNetMap.value("1", "0"));
                QString n2 = normalizeNode(comp.pinToNetMap.value("2", "0"));
                deck << QString("%1 %2 %3 %4").arg(comp.refDes, n1, n2, comp.value);
            }
            if (!comp.spiceModel.inlineDefinition.isEmpty()) {
                deck << comp.spiceModel.inlineDefinition;
            }
        }

        for (auto it = sourceValues.begin(); it != sourceValues.end(); ++it) {
            deck << QString("V_%1 %2 0 DC %3").arg(it.key().toUpper(), normalizeNode(it.key())).arg(it.value());
        }

        deck << ".END";
        return deck.join("\n");
    }

    SimulationResult runOperatingPoint(const Netlist& netlist, const QMap<QString, double>& sourceValues = {}) {
        return executeAnalysis(netlist, sourceValues, "op");
    }

    SimulationResult runDC(const Netlist& netlist, const QMap<QString, double>& sourceValues) {
        return executeAnalysis(netlist, sourceValues, "op");
    }

    SimulationResult runTransient(const Netlist& netlist, double tStep, double tStop) {
        return executeAnalysis(netlist, {}, QString("tran %1 %2").arg(tStep).arg(tStop));
    }

    SimulationResult runAC(const Netlist& netlist, double fStart, double fStop) {
        return executeAnalysis(netlist, {}, QString("ac dec 10 %1 %2").arg(fStart).arg(fStop));
    }

private:
    QString lastLogs_;

    SimulationResult executeAnalysis(const Netlist& netlist, const QMap<QString, double>& sourceValues, const QString& analysisCmd) {
        SimulationResult res;
        lastLogs_.clear();

        QString deck = generateSpiceDeck(netlist, sourceValues);
        ngSpice_Command(const_cast<char*>("destroy all"));
        ngSpice_Command(const_cast<char*>("remcirc"));

        QStringList lines = deck.split("\n");
        char** circArray = new char*[lines.size() + 1];
        for (int i = 0; i < lines.size(); ++i) {
            QByteArray ba = lines[i].toLocal8Bit();
            circArray[i] = strdup(ba.constData());
        }
        circArray[lines.size()] = nullptr;

        if (ngSpice_Circ(circArray) != 0) {
            res.errorMessage = "Failed to load SPICE circuit:\n" + lastLogs_;
            freeCircArray(circArray, lines.size());
            return res;
        }
        freeCircArray(circArray, lines.size());

        for (const auto& comp : netlist.components) {
            QString saveCmd = QString("save @%1[i] @%1[p]").arg(comp.refDes.toLower());
            ngSpice_Command(saveCmd.toLocal8Bit().data());
        }

        if (ngSpice_Command(analysisCmd.toLocal8Bit().data()) != 0) {
            res.errorMessage = "Analysis failed:\n" + lastLogs_;
            return res;
        }

        char* curPlot = ngSpice_CurPlot();
        if (!curPlot) return res;
        char** allVecs = ngSpice_AllVecs(curPlot);
        if (!allVecs) return res;

        for (int i = 0; allVecs[i] != nullptr; ++i) {
            pvector_info vec = ngGet_Vec_Info(allVecs[i]);
            if (!vec || vec->v_length <= 0 || !vec->v_realdata) continue;

            QString vecName = QString::fromLocal8Bit(vec->v_name);
            double val = vec->v_realdata[vec->v_length - 1];

            if (vecName.startsWith("@") && vecName.endsWith("[i]")) {
                QString ref = vecName.mid(1, vecName.length() - 4).toUpper();
                res.branchCurrents[ref] = val;
            } else if (vecName.startsWith("@") && vecName.endsWith("[p]")) {
                QString ref = vecName.mid(1, vecName.length() - 4).toUpper();
                res.componentPowers[ref] = val;
            } else if (vecName.endsWith("#branch")) {
                res.branchCurrents[vecName.left(vecName.indexOf('#')).toUpper()] = val;
            } else {
                res.nodeVoltages[vecName.toUpper()] = val;
            }
        }

        res.success = true;
        return res;
    }

    static QString normalizeNode(const QString& netName) {
        if (netName.compare("GND", Qt::CaseInsensitive) == 0 || netName == "0")
            return "0";
        return netName;
    }

    static void freeCircArray(char** arr, int size) {
        for (int i = 0; i < size; ++i) free(arr[i]);
        delete[] arr;
    }

    static int ngSendChar(char* output, int, void* userData) {
        auto* self = static_cast<SpiceSimulator*>(userData);
        self->lastLogs_ += QString::fromLocal8Bit(output) + "\n";
        return 0;
    }
    static int ngSendStat(char*, int, void*) { return 0; }
    static int ngControlledExit(int, bool, bool, int, void*) { return 0; }
    static int ngBGThreadRunning(bool, int, void*) { return 0; }
};

} // namespace CircuitGuard
