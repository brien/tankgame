#include "GeometryAsset.h"
#include "GpuGeometry.h"

#include <stdexcept>

struct GeometryAsset::Storage {
    std::unique_ptr<GpuGeometry> geometry;
};

GeometryAsset::GeometryAsset() : storage(std::make_shared<Storage>()) {}

void GeometryAsset::SetGeometry(const Geometry& geometry)
{
    // Prepare before replacing the shared upload so failures preserve it.
    auto uploaded = std::make_unique<GpuGeometry>(geometry);
    if (!storage) storage = std::make_shared<Storage>();
    storage->geometry = std::move(uploaded);
}

const GpuGeometry& GeometryAsset::GetGpuGeometry() const
{
    if (!storage || !storage->geometry)
        throw std::logic_error("GeometryAsset has no uploaded geometry");
    return *storage->geometry;
}
