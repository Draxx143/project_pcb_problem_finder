// Drc.h — Industrial Design Rule Check (DRC) & DFM Engineering Report Engine for CircuitGuard Pro.
#pragma once

#include <string>
#include <vector>
#include "Model.h"

namespace pcbv {

enum class DrcSeverity {
    Critical, // Short circuit, drill break-out, missing net
    Warning,  // Clearance violation, thin power trace, small annular ring
    Info      // DFM recommendation, unplated hole notice
};

struct DrcRules {
    double minTrackWidthMil = 6.0;       // IPC Class 2 standard (6 mil)
    double minPowerTrackWidthMil = 15.0; // Power rails (VCC, GND, 5V, 3V3...)
    double minViaHoleMil = 8.0;          // Minimum mechanical drill
    double minAnnularRingMil = 4.0;      // Via/Pad annular ring
    double minClearanceMil = 6.0;        // Copper-to-copper clearance
};

struct DrcViolation {
    DrcSeverity severity = DrcSeverity::Warning;
    std::string code;        // e.g. "DRC-CLR-01", "DRC-TRK-02"
    std::string title;
    std::string description;
    std::string net;
    std::string location;    // e.g. "(1250.0, 840.5) mil"
    double x = 0.0;
    double y = 0.0;
};

struct DrcReport {
    int criticalCount = 0;
    int warningCount = 0;
    int infoCount = 0;
    double healthScore = 100.0; // 0..100 industrial manufacturing readiness score
    std::vector<DrcViolation> violations;
};

/// Runs industrial DRC/DFM checks on a parsed PCB document.
DrcReport runDrcChecks(const Document& doc, const DrcRules& rules = DrcRules{});

/// Generates a commercial dark-themed HTML/Printable DFM & DRC Engineering Report.
std::string generateCommercialDrcReportHtml(const Document& doc, const DrcReport& report);

} // namespace pcbv
