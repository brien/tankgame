# `igtl_QGLMesh` CPU geometry migration

## Audited emission variants

The mesh API previously exposed seven effective immediate-mode variants. Repository-wide
call-site searches found the following consumers at the migration point:

| Legacy behavior | CPU geometry method | Topology | Repository consumers |
|---|---|---|---|
| ordinary triangles | `CreateTriangleGeometry` | triangles | tank body/turret/cannon lists in `GraphicsTask` and `ResourceManager` |
| facet-offset triangles | `CreateTriangleExtrudedGeometry` | triangles | enhanced tank and item lists in `GraphicsTask`; equivalent resources in `ResourceManager` |
| colored triangles | `CreateTriangleColoredGeometry` | triangles | none |
| colored, facet-offset triangles | `CreateTriangleColoredExtrudedGeometry` | triangles | none |
| edges | `CreateEdgeGeometry` | independent lines | none (one item call was commented out) |
| edge-normal-offset edges | `CreateEdgeExtrudedGeometry` | independent lines | tank outline lists in `GraphicsTask` and `ResourceManager` |
| colored edges / colored edge-normal-offset edges | `CreateEdgeColoredGeometry` / `CreateEdgeColoredExtrudedGeometry` | independent lines | none |

All variants are now extraction methods rather than drawing methods. The unused colored
variants were retained because color and line topology fit the same existing `Geometry`
record without a separate abstraction. There are no remaining legacy mesh drawing
methods.

## Preserved contracts

Triangle extraction emits three duplicated vertices per source face in `v1`, `v2`,
`v3` order. Each output carries the source vertex normal and UV. Colored variants copy
the face color onto all three vertices. “Extrusion” retains its historical meaning: it
only adds `amount * facetNormal` to each position; it creates neither a back face nor
side faces.

Edge extraction emits two duplicated vertices per enabled (`m_flag != 0`) edge in
`v1`, `v2` order. Each output carries the source vertex normal and UV. Colored variants
copy the edge color to both endpoints. Extruded edge variants add
`amount * edgeNormal` to both endpoints. Despite old comments claiming all edge methods
used colors, the two non-colored variants did not submit colors; the CPU metadata
preserves that distinction. Edge indices remain stored as floats by the legacy file
format/class and are converted to unsigned indices exactly where the old vector access
implicitly converted them.

`Geometry` gained only the `LINES` topology. Existing color fields and
`hasColors`/`hasNormals`/`hasTextureCoordinates` presence flags represent all other
attributes. The native `DisplayList` adapter submits lines through `GL_LINES`; the
Emscripten side constructs and retains the CPU record without attempting unsupported
drawing.
