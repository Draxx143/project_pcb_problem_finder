#pragma once
#include "CoreDataStructures.h"
#include <QProcess>
#include <QTemporaryFile>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>

namespace CircuitGuard {

class SpiceNetlistParser : public INetlistProvider {
public:
    bool loadFromFile(const QString& filePath, Netlist& outNetlist, QString& errorMsg) override {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            errorMsg = "Cannot open SPICE file: " + filePath;
            return false;
        }
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (line.isEmpty() || line.startsWith("*") || line.startsWith(".")) continue;
            QStringList tokens = line.split(QRegularExpression("\\s+"));
            if (tokens.size() >= 4 && tokens[0].startsWith("R", Qt::CaseInsensitive)) {
                ComponentInstance comp;
                comp.refDes = tokens[0].toUpper();
                comp.spiceModel.type = SpiceModel::Type::Resistor;
                comp.pinToNetMap["1"] = tokens[1].toUpper();
                comp.pinToNetMap["2"] = tokens[2].toUpper();
                comp.value = tokens[3];
                outNetlist.components[comp.refDes] = comp;
                outNetlist.nets[tokens[1].toUpper()].name = tokens[1].toUpper();
                outNetlist.nets[tokens[1].toUpper()].connectedPins.append({comp.refDes, "1"});
                outNetlist.nets[tokens[2].toUpper()].name = tokens[2].toUpper();
                outNetlist.nets[tokens[2].toUpper()].connectedPins.append({comp.refDes, "2"});
            }
        }
        return true;
    }
};

class KiCadNetlistProvider : public INetlistProvider {
public:
    bool loadFromFile(const QString& schFilePath, Netlist& outNetlist, QString& errorMsg) override {
        QTemporaryFile tempSpFile("XXXXXX.cir");
        if (!tempSpFile.open()) {
            errorMsg = "Cannot create temporary file.";
            return false;
        }
        QString outPath = tempSpFile.fileName();
        tempSpFile.close();

        QProcess kicadCli;
        kicadCli.start("kicad-cli", {"sch", "export", "netlist", "--format", "spice", "-o", outPath, schFilePath});
        if (!kicadCli.waitForFinished(10000) || kicadCli.exitCode() != 0) {
            errorMsg = "kicad-cli failed: " + kicadCli.readAllStandardError();
            return false;
        }
        SpiceNetlistParser spiceParser;
        return spiceParser.loadFromFile(outPath, outNetlist, errorMsg);
    }
};

} // namespace CircuitGuard
