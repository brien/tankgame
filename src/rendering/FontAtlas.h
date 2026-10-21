#pragma once

#include "ImageData.h"
#include "../Geometry.h"
#include <array>

// CPU description retained separately from GPU resources. The diagnostic UI uses
// printable ASCII; unsupported bytes use '?' rather than indexing outside it.
struct FontGlyph {
    Geometry geometry;
    int width = 0;
    int height = 0;
    int advance = 0;
};
struct FontAtlas {
    static constexpr int First = 32;
    static constexpr int Count = 95;
    ImageData image;
    std::array<FontGlyph, Count> glyphs;
    int lineHeight = 0;
    static FontAtlas Load(const char* path);
    static int GlyphIndex(unsigned char character) {
        return character >= First && character < First + Count ? character - First : '?' - First;
    }
};
