#ifndef EFFECTDATAEXTRACTOR_H
#define EFFECTDATAEXTRACTOR_H

#include <vector>
#include "RenderData.h"

// Forward declarations
class FX;

/**
 * Utility class for extracting rendering data from effect objects
 * Provides data conversion methods for the new rendering pipeline
 */
class EffectDataExtractor {
public:
    /**
     * Extract rendering data from a vector of FX objects
     * @param effects Vector of FX objects to extract data from
     * @return Vector of EffectRenderData for rendering
     */
    static std::vector<EffectRenderData> ExtractEffectRenderData(const std::vector<FX>& effects);
    
    /**
     * Extract rendering data from a single FX object
     * @param effect FX object to extract data from
     * @return EffectRenderData for rendering
     */
    static EffectRenderData ExtractSingleEffectData(const FX& effect);
};

#endif // EFFECTDATAEXTRACTOR_H
