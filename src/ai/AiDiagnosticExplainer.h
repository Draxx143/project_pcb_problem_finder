#pragma once
#include "../compiler/CircuitCompiler.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QEventLoop>

namespace CircuitGuard {

class AiDiagnosticExplainer {
public:
    static QString explainDiagnostic(const CompilationDiagnostic& diag,
                                     const Netlist& netlist,
                                     const SimulationResult& sim) {
        QStringList neighborDetails;
        QVector<QString> adjacentComps = netlist.getAdjacentComponents(diag.targetObject);

        for (const QString& ref : adjacentComps) {
            const auto& comp = netlist.components[ref];
            double currentmA = std::abs(sim.branchCurrents.value(ref, 0.0)) * 1000.0;
            double powermW = std::abs(sim.componentPowers.value(ref, 0.0)) * 1000.0;
            neighborDetails << QString("- %1 (Value: %2): Current = %3 mA, Power = %4 mW")
                               .arg(ref, comp.value)
                               .arg(currentmA, 0, 'f', 2)
                               .arg(powermW, 0, 'f', 2);
        }

        QString prompt = QString(
            "You are an electronic circuit compiler diagnostic engine.\n"
            "Rule Violated: %1\n"
            "Target Net/Component: %2\n"
            "Symptom: %3\n"
            "Connected Components & Simulated Metrics:\n%4\n\n"
            "Explain specifically WHICH component value or missing connection caused this mismatch in 2 short Persian sentences starting with 'دلیل احتمالی:'."
        ).arg(diag.ruleId, diag.targetObject, diag.rawMessage, neighborDetails.join("\n"));

        return queryOllama(prompt);
    }

private:
    static QString queryOllama(const QString& prompt) {
        QNetworkAccessManager mgr;
        QNetworkRequest req(QUrl("http://localhost:11434/api/generate"));
        req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        QJsonObject payload;
        payload["model"] = "qwen2.5-coder:7b";
        payload["prompt"] = prompt;
        payload["stream"] = false;

        QNetworkReply* reply = mgr.post(req, QJsonDocument(payload).toJson());
        QEventLoop loop;
        QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();

        if (reply->error() != QNetworkReply::NoError) {
            reply->deleteLater();
            return "دلیل احتمالی: مقدار مقاومت سری بیش از حد بالاست یا یکی از انشعاب‌های بار قطع شده است. (سرویس Ollama در دسترس نیست)";
        }

        QJsonDocument respDoc = QJsonDocument::fromJson(reply->readAll());
        reply->deleteLater();
        return respDoc.object().value("response").toString().trimmed();
    }
};

} // namespace CircuitGuard
