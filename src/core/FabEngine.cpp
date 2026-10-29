// FabEngine.cpp — Implementation of Fab outputs and cross-checks.
#include "FabEngine.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <map>
#include <set>

namespace pcbv {

std::string generatePickAndPlaceCsv(const Document& doc) {
    std::ostringstream ss;
    ss << "Designator,Footprint,Layer,Mid X (mm),Mid Y (mm),Rotation,Comment\n";
    for (const auto& c : doc.components) {
        double xMm = c.x * 0.0254;
        double yMm = c.y * 0.0254;
        ss << c.designator << ","
           << "\"" << c.footprint << "\","
           << (c.layer.empty() ? "TOP" : c.layer) << ","
           << std::fixed << std::setprecision(3) << xMm << ","
           << yMm << ","
           << c.rotation << ","
           << "\"" << c.comment << "\"\n";
    }
    return ss.str();
}

std::string generateGroupedBomCsv(const Document& doc) {
    std::ostringstream ss;
    ss << "Designator(s),Quantity,Footprint,Comment,Description\n";
    std::map<std::string, std::vector<std::string>> groups;
    std::map<std::string, Component> prototypes;

    for (const auto& c : doc.components) {
        std::string key = c.footprint + "|" + c.comment;
        groups[key].push_back(c.designator);
        if (prototypes.find(key) == prototypes.end()) {
            prototypes[key] = c;
        }
    }

    for (const auto& pair : groups) {
        const auto& key = pair.first;
        const auto& desgs = pair.second;
        const auto& proto = prototypes[key];

        ss << "\"";
        for (std::size_t i = 0; i < desgs.size(); ++i) {
            ss << desgs[i] << (i + 1 < desgs.size() ? ", " : "");
        }
        ss << "\","
           << desgs.size() << ","
           << "\"" << proto.footprint << "\","
           << "\"" << proto.comment << "\","
           << "\"" << proto.description << "\"\n";
    }
    return ss.str();
}

SchematicPcbCheckResult validateSchVsPcb(const Document& schDoc, const Document& pcbDoc) {
    SchematicPcbCheckResult res;
    res.schematicComponents = static_cast<int>(schDoc.components.size());
    res.pcbComponents = static_cast<int>(pcbDoc.components.size());

    std::set<std::string> schSet, pcbSet;
    for (const auto& c : schDoc.components) schSet.insert(c.designator);
    for (const auto& c : pcbDoc.components) pcbSet.insert(c.designator);

    for (const auto& des : schSet) {
        if (pcbSet.find(des) == pcbSet.end()) {
            res.missingOnPcb.push_back(des);
        }
    }
    for (const auto& des : pcbSet) {
        if (schSet.find(des) == schSet.end()) {
            res.extraOnPcb.push_back(des);
        }
    }

    res.valid = res.missingOnPcb.empty() && res.extraOnPcb.empty();
    std::ostringstream sum;
    if (res.valid) {
        sum << "✅ Schematic and PCB match perfectly (" << res.schematicComponents << " components verified).";
    } else {
        sum << "⚠️ Mismatch detected! Sch components: " << res.schematicComponents << ", PCB components: " << res.pcbComponents;
        if (!res.missingOnPcb.empty()) sum << " | Missing on PCB: " << res.missingOnPcb.size();
        if (!res.extraOnPcb.empty()) sum << " | Extra on PCB: " << res.extraOnPcb.size();
    }
    res.summary = sum.str();
    return res;
}

} // namespace pcbv
