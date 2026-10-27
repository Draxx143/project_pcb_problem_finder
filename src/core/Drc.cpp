// Drc.cpp — Industrial DRC & Commercial Report Generator implementation.
#include "Drc.h"
#include <cmath>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace pcbv {

namespace {
bool isPowerNet(const std::string& net) {
    std::string u = net;
    std::transform(u.begin(), u.end(), u.begin(), ::toupper);
    return u == "GND" || u == "VCC" || u == "VDD" || u == "VSS" ||
           u.find("5V") != std::string::npos || u.find("3V3") != std::string::npos ||
           u.find("12V") != std::string::npos || u.find("PWR") != std::string::npos;
}

std::string fmtCoord(double x, double y) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << "(" << x << ", " << y << ") mil";
    return oss.str();
}
} // namespace

DrcReport runDrcChecks(const Document& doc, const DrcRules& rules) {
    DrcReport rep;
    if (doc.type != DocType::PcbDoc) return rep;

    // 1. Check Tracks (Width & Power Rail Capacity)
    for (const auto& t : doc.geometry.tracks) {
        if (t.width > 0.0 && t.width < rules.minTrackWidthMil) {
            rep.violations.push_back({
                DrcSeverity::Warning, "DRC-TRK-01", "Minimum Track Width Violation",
                "Track width (" + std::to_string(t.width) + " mil) is below IPC limit (" + std::to_string(rules.minTrackWidthMil) + " mil).",
                t.net, fmtCoord(t.x1, t.y1), t.x1, t.y1
            });
        } else if (!t.net.empty() && isPowerNet(t.net) && t.width > 0.0 && t.width < rules.minPowerTrackWidthMil) {
            rep.violations.push_back({
                DrcSeverity::Info, "DFM-PWR-01", "Narrow Power Rail Trace",
                "Power net "" + t.net + "" routed with " + std::to_string(t.width) + " mil width (recommended >= 15 mil).",
                t.net, fmtCoord(t.x1, t.y1), t.x1, t.y1
            });
        }
    }

    // 2. Check Vias (Drill Size & Annular Ring Break-out)
    for (const auto& v : doc.geometry.vias) {
        if (v.hole > 0.0 && v.hole < rules.minViaHoleMil) {
            rep.violations.push_back({
                DrcSeverity::Warning, "DRC-VIA-01", "Micro-Drill Size Warning",
                "Via drill hole (" + std::to_string(v.hole) + " mil) requires high-cost laser/micro drilling.",
                v.net, fmtCoord(v.x, v.y), v.x, v.y
            });
        }
        double annular = (v.diameter - v.hole) * 0.5;
        if (v.diameter > 0.0 && annular < rules.minAnnularRingMil) {
            rep.violations.push_back({
                annular <= 1.0 ? DrcSeverity::Critical : DrcSeverity::Warning,
                "DRC-VIA-02", "Insufficient Via Annular Ring",
                "Annular ring is " + std::to_string(annular) + " mil (risk of drill breakout during fabrication).",
                v.net, fmtCoord(v.x, v.y), v.x, v.y
            });
        }
    }

    // 3. Check Pad-to-Pad Short Circuit / Clearance (Sampled spatial check)
    const auto& pads = doc.geometry.pads;
    std::size_t maxCheck = std::min<std::size_t>(pads.size(), 600);
    for (std::size_t i = 0; i < maxCheck; ++i) {
        for (std::size_t j = i + 1; j < maxCheck; ++j) {
            if (pads[i].layer != pads[j].layer) continue;
            if (pads[i].net.empty() || pads[j].net.empty() || pads[i].net == pads[j].net) continue;
            double dx = pads[i].x - pads[j].x;
            double dy = pads[i].y - pads[j].y;
            double centerDist = std::hypot(dx, dy);
            double r1 = std::max(pads[i].width, pads[i].height) * 0.5;
            double r2 = std::max(pads[j].width, pads[j].height) * 0.5;
            double edgeGap = centerDist - (r1 + r2);
            if (edgeGap < 0.0) {
                rep.violations.push_back({
                    DrcSeverity::Critical, "DRC-SHT-01", "Copper Short Circuit Detected",
                    "Pad " + pads[i].designator + " (" + pads[i].net + ") overlaps Pad " + pads[j].designator + " (" + pads[j].net + ").",
                    pads[i].net, fmtCoord(pads[i].x, pads[i].y), pads[i].x, pads[i].y
                });
            } else if (edgeGap < rules.minClearanceMil) {
                rep.violations.push_back({
                    DrcSeverity::Warning, "DRC-CLR-01", "Pad-to-Pad Clearance Violation",
                    "Clearance between " + pads[i].designator + " and " + pads[j].designator + " is below " + std::to_string(rules.minClearanceMil) + " mil.",
                    pads[i].net, fmtCoord(pads[i].x, pads[i].y), pads[i].x, pads[i].y
                });
            }
        }
    }

    // 4. Check Unrouted / Single-Node Nets
    for (const auto& ns : doc.netSummaries) {
        if (ns.pads >= 2 && ns.tracks == 0) {
            rep.violations.push_back({
                DrcSeverity::Critical, "DRC-NET-01", "Unrouted Net (Open Circuit)",
                "Net "" + ns.name + "" connects " + std::to_string(ns.pads) + " pads but has 0 routed copper tracks.",
                ns.name, "Global Netlist", 0.0, 0.0
            });
        }
    }

    for (const auto& v : rep.violations) {
        if (v.severity == DrcSeverity::Critical) rep.criticalCount++;
        else if (v.severity == DrcSeverity::Warning) rep.warningCount++;
        else rep.infoCount++;
    }

    double penalty = rep.criticalCount * 15.0 + rep.warningCount * 3.0 + rep.infoCount * 0.5;
    rep.healthScore = std::max(0.0, 100.0 - penalty);
    return rep;
}

std::string generateCommercialDrcReportHtml(const Document& doc, const DrcReport& rep) {
    std::ostringstream h;
    std::string scoreColor = rep.healthScore >= 85.0 ? "#00e676" : (rep.healthScore >= 60.0 ? "#ffab00" : "#ff1744");

    h << R"(<!DOCTYPE html><html lang="en"><head><meta charset="utf-8">
<title>CircuitGuard Pro — Engineering DRC & DFM Report</title>
<style>
  body { background:#0d1117; color:#e6edf3; font-family:"Segoe UI",Roboto,sans-serif; margin:32px; }
  .header { display:flex; justify-content:space-between; align-items:center; border-bottom:2px solid #00e5ff; padding-bottom:16px; }
  .brand { font-size:26px; font-weight:800; color:#00e5ff; letter-spacing:1px; }
  .sub { color:#8b949e; font-size:13px; }
  .cards { display:flex; gap:16px; margin:24px 0; }
  .card { background:#161b22; border:1px solid #30363d; border-radius:8px; padding:16px 24px; flex:1; }
  .card h3 { margin:0 0 6px 0; font-size:13px; color:#8b949e; text-transform:uppercase; }
  .card .val { font-size:28px; font-weight:700; }
  table { width:100%; border-collapse:collapse; background:#161b22; border-radius:8px; overflow:hidden; }
  th { background:#21262d; color:#00e5ff; text-align:left; padding:12px; font-size:13px; }
  td { padding:10px 12px; border-top:1px solid #30363d; font-size:13px; }
  .crit { color:#ff5252; font-weight:700; }
  .warn { color:#ffd740; font-weight:600; }
  .info { color:#40c4ff; }
</style></head><body>
<div class="header">
  <div><div class="brand">🛡️ CIRCUITGUARD PRO</div><div class="sub">Industrial Design Rule Check (DRC) & DFM Sign-Off Report</div></div>
  <div class="sub">Board: <b>)" << doc.fileName << R"(</b><br>Size: )" << std::fixed << std::setprecision(1) << doc.bounds.width() << " x " << doc.bounds.height() << R"( mil</div>
</div>
<div class="cards">
  <div class="card"><h3>DFM Health Score</h3><div class="val" style="color:)" << scoreColor << R"(">)" << std::fixed << std::setprecision(0) << rep.healthScore << R"(%</div></div>
  <div class="card"><h3>Critical Errors</h3><div class="val crit">)" << rep.criticalCount << R"(</div></div>
  <div class="card"><h3>Warnings</h3><div class="val warn">)" << rep.warningCount << R"(</div></div>
  <div class="card"><h3>Components / Nets</h3><div class="val">)" << doc.components.size() << " / " << doc.nets.size() << R"(</div></div>
</div>
<table><thead><tr><th>Severity</th><th>Code</th><th>Issue</th><th>Net</th><th>Location</th><th>Details</th></tr></thead><tbody>)";

    if (rep.violations.empty()) {
        h << R"(<tr><td colspan="6" style="text-align:center;color:#00e676;padding:24px;">✅ Zero DRC Violations Detected — Board is Ready for Manufacturing!</td></tr>)";
    } else {
        for (const auto& v : rep.violations) {
            std::string cls = v.severity == DrcSeverity::Critical ? "crit" : (v.severity == DrcSeverity::Warning ? "warn" : "info");
            std::string lbl = v.severity == DrcSeverity::Critical ? "CRITICAL" : (v.severity == DrcSeverity::Warning ? "WARNING" : "INFO");
            h << "<tr><td class="" << cls << "">" << lbl << "</td><td><code>" << v.code << "</code></td><td><b>"
              << v.title << "</b></td><td>" << (v.net.empty() ? "-" : v.net) << "</td><td>" << v.location
              << "</td><td>" << v.description << "</td></tr>
";
        }
    }
    h << "</tbody></table></body></html>";
    return h.str();
}

} // namespace pcbv
