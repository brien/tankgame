#ifndef BULLETDATAEXTRACTOR_H
#define BULLETDATAEXTRACTOR_H

#include <vector>
#include "RenderData.h"

// Forward declarations
class Bullet;

/**
 * Utility class for extracting rendering data from bullet objects
 * Provides data conversion methods for the new rendering pipeline
 */
class BulletDataExtractor {
public:
    /**
     * Extract rendering data from a vector of Bullet objects
     * @param bullets Vector of bullet objects to extract data from
     * @return Vector of BulletRenderData for rendering
     */
    static std::vector<BulletRenderData> ExtractBulletRenderData(const std::vector<Bullet>& bullets);
    
    /**
     * Extract rendering data from a single Bullet object
     * @param bullet Bullet object to extract data from
     * @return BulletRenderData for rendering
     */
    static BulletRenderData ExtractSingleBulletData(const Bullet& bullet);
};

#endif // BULLETDATAEXTRACTOR_H
