// FabEngine.h — Industrial Manufacturing Package & Cross-Check Engine for CircuitGuard Pro.
#pragma once

#include <string>
#include <vector>
#include "Model.h"

namespace pcbv {

/// Generates standard Pick & Place (Centroid CSV) for SMT assembly.
std::string generatePickAndPlaceCsv(const Document& doc);

/// Generates a complete Bill of Materials (BOM) grouped CSV.
std::string generateGroupedBomCsv(const Document& doc);

/// Validates Schematic (SchDoc) vs PCB (PcbDoc) connectivity & component count.
struct SchematicPcbCheckResult {
    bool valid = true;
    int schematicComponents = 0;
    int pcbComponents = 0;
    std::vector<std::string>missingOnPcb;
    std::vector<std::string>extraOnPcb;
    std::string summary;
};

SchematicPcbCheckResult validateSchVsPcb(const Document& schDoc, const Document& pcbDoc);

} // namespace pcbv
