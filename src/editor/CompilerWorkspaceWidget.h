#pragma once
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QGraphicsView>
#include <QTextEdit>
#include <QSplitter>
#include <QLabel>
#include "SchematicScene.h"
#include "../compiler/CircuitCompiler.h"
#include "../ai/AiDiagnosticExplainer.h"

namespace CircuitGuard {

class CompilerWorkspaceWidget : public QWidget {
    Q_OBJECT
public:
    explicit CompilerWorkspaceWidget(QWidget* parent = nullptr) : QWidget(parent) {
        auto* mainLayout = new QVBoxLayout(this);

        // نوار ابزار صنعتی بالای ویرایشگر
        auto* toolbarLayout = new QHBoxLayout();
        auto* btnAddResistor = new QPushButton("+ Add Resistor (R12 = 66Ω)", this);
        auto* btnCompile = new QPushButton("⚡ Compile & Run SPICE Rules", this);
        btnCompile->setStyleSheet("background-color: #2ea043; color: white; font-weight: bold; padding: 6px 14px;");

        toolbarLayout->addWidget(btnAddResistor);
        toolbarLayout->addWidget(btnCompile);
        toolbarLayout->addStretch();
        mainLayout->addLayout(toolbarLayout);

        // اسپلیتر بوم شماتیک و پنل پیام‌های کامپایلر (شبیه پنل Messages آلتیوم)
        auto* splitter = new QSplitter(Qt::Vertical, this);

        scene_ = new SchematicScene(this);
        scene_->setSceneRect(0, 0, 800, 500);
        view_ = new QGraphicsView(scene_, this);
        view_->setRenderHint(QPainter::Antialiasing);
        view_->setStyleSheet("background-color: #1e1e1e; border: 1px solid #3c3c3c;");
        splitter->addWidget(view_);

        diagnosticsOutput_ = new QTextEdit(this);
        diagnosticsOutput_->setReadOnly(true);
        diagnosticsOutput_->setStyleSheet("background-color: #141414; color: #e0e0e0; font-family: monospace; padding: 6px;");
        diagnosticsOutput_->setPlaceholderText("Compiler & SPICE ERC Diagnostics will appear here...");
        splitter->addWidget(diagnosticsOutput_);
        splitter->setStretchFactor(0, 3);
        splitter->setStretchFactor(1, 1);

        mainLayout->addWidget(splitter);

        // بارگذاری مدار نمونه اولیه روی صفحه شماتیک
        loadSampleCircuit();

        connect(btnAddResistor, &QPushButton::clicked, this, &CompilerWorkspaceWidget::loadSampleCircuit);
        connect(btnCompile, &QPushButton::clicked, this, &CompilerWorkspaceWidget::runCircuitCompilation);
    }

public slots:
    void loadSampleCircuit() {
        scene_->clear();
        ComponentInstance r12;
        r12.refDes = "R12";
        r12.value = "66";
        r12.spiceModel.type = SpiceModel::Type::Resistor;
        r12.ratings.maxPower = 0.25;
        scene_->addSymbol(r12, QPointF(200, 150));
        diagnosticsOutput_->append("[INFO] Sample component R12 (66 Ohm) placed on Schematic Canvas.");
    }

    void runCircuitCompilation() {
        diagnosticsOutput_->clear();
        diagnosticsOutput_->append("<b>=== Running CircuitGuard SPICE Compiler ===</b>");

        Netlist netlist = scene_->extractNetlistFromCanvas();
        if (netlist.components.contains("R12")) {
            netlist.components["R12"].pinToNetMap["1"] = "VCC_3V3";
            netlist.components["R12"].pinToNetMap["2"] = "GND";
            Net vccNet;
            vccNet.name = "VCC_3V3";
            vccNet.connectedPins.append({"R12", "1"});
            vccNet.constraint = ExpectedConstraint{"VCC_3V3", 3.3, 5.0, 0.300, 0.500};
            netlist.nets["VCC_3V3"] = vccNet;
            Net gndNet;
            gndNet.name = "GND";
            gndNet.connectedPins.append({"R12", "2"});
            netlist.nets["GND"] = gndNet;
        }

        CircuitCompiler compiler;
        SimulationResult simResult;
        auto diagnostics = compiler.compile(netlist, {{"VCC_3V3", 3.3}}, simResult);

        if (diagnostics.isEmpty()) {
            diagnosticsOutput_->append("<span style='color:#4ec9b0;'>[PASS] All electrical and physical rules passed!</span>");
            return;
        }

        for (auto& diag : diagnostics) {
            diag.aiRootCauseExplanation = AiDiagnosticExplainer::explainDiagnostic(diag, netlist, simResult);
            diagnosticsOutput_->append(QString("<span style='color:#f44747;'><b>[%1]</b> %2</span>")
                                       .arg(diag.ruleId, diag.rawMessage));
            diagnosticsOutput_->append(QString("<span style='color:#dcdcaa;'>&nbsp;&nbsp;↳ %1</span><br>")
                                       .arg(diag.aiRootCauseExplanation));
        }
    }

private:
    SchematicScene* scene_ = nullptr;
    QGraphicsView* view_ = nullptr;
    QTextEdit* diagnosticsOutput_ = nullptr;
};

} // namespace CircuitGuard
