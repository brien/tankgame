#pragma once

#include <memory>
#include "../Geometry.h"

class GpuGeometry;

// Copyable catalogue entry. Copies share uploads; resetting one entry leaves
// other owners valid. Construction is CPU-only. Uploads and final destruction
// require a current GL context, like GpuGeometry itself.
class GeometryAsset
{
public:
    GeometryAsset();
    void SetGeometry(const Geometry& geometry);
    const GpuGeometry& GetGpuGeometry() const;

private:
    struct Storage;
    std::shared_ptr<Storage> storage;
};
