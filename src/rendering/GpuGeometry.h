#pragma once

#include <memory>

#include "../GeometryBuffer.h"

// Immutable uploaded geometry for the programmable renderer.  It owns only
// buffer/layout state; programs, uniforms and materials belong to submission.
class GpuGeometry
{
public:
    explicit GpuGeometry(const Geometry& geometry);
    ~GpuGeometry();
    GpuGeometry(const GpuGeometry&) = delete;
    GpuGeometry& operator=(const GpuGeometry&) = delete;

    const GeometryAttributeLayout& Layout() const;
    std::size_t VertexCount() const;
    GeometryDrawMode DrawMode() const;

private:
    unsigned int BufferHandle() const;
    class Implementation;
    std::unique_ptr<Implementation> implementation;
    friend class ModernRenderer;
};
