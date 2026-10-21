#include "FontAtlas.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <cstring>

FontAtlas FontAtlas::Load(const char* path) {
    if (TTF_Init() != 0) throw std::runtime_error(TTF_GetError());
    struct Quit { ~Quit() { TTF_Quit(); } } quit;
    std::unique_ptr<TTF_Font, decltype(&TTF_CloseFont)> font(TTF_OpenFont(path, 32), TTF_CloseFont);
    if (!font) throw std::runtime_error(TTF_GetError());
    using Surface = std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)>;
    std::vector<Surface> surfaces;
    int cellWidth = 0, cellHeight = 0;
    FontAtlas atlas;
    atlas.lineHeight = TTF_FontLineSkip(font.get());
    for (int i = 0; i < Count; ++i) {
        const char text[] = {static_cast<char>(First + i), 0};
        Surface original(TTF_RenderText_Blended(font.get(), text, SDL_Color{255,255,255,255}), SDL_FreeSurface);
        if (!original) throw std::runtime_error(TTF_GetError());
        Surface surface(SDL_ConvertSurfaceFormat(original.get(), SDL_PIXELFORMAT_RGBA32, 0), SDL_FreeSurface);
        if (!surface) throw std::runtime_error(SDL_GetError());
        auto& glyph = atlas.glyphs[i];
        glyph.width = surface->w; glyph.height = surface->h;
        if (TTF_GlyphMetrics(font.get(), First + i, nullptr, nullptr, nullptr, nullptr, &glyph.advance) != 0)
            throw std::runtime_error(TTF_GetError());
        cellWidth = std::max(cellWidth, glyph.width + 2);
        cellHeight = std::max(cellHeight, glyph.height + 2);
        surfaces.push_back(std::move(surface));
    }
    atlas.image.width = cellWidth * 16;
    atlas.image.height = cellHeight * 6;
    atlas.image.format = PixelFormat::RGBA8;
    atlas.image.pixels.resize(atlas.image.width * atlas.image.height * 4, 0);
    for (int i = 0; i < Count; ++i) {
        const int x = (i % 16) * cellWidth + 1, y = (i / 16) * cellHeight + 1;
        auto& glyph = atlas.glyphs[i];
        for (int row = 0; row < glyph.height; ++row)
            std::memcpy(&atlas.image.pixels[((y + row) * atlas.image.width + x) * 4],
                static_cast<const unsigned char*>(surfaces[i]->pixels) + row * surfaces[i]->pitch, glyph.width * 4);
        const float u0 = float(x) / atlas.image.width, u1 = float(x + glyph.width) / atlas.image.width;
        const float v0 = float(y) / atlas.image.height, v1 = float(y + glyph.height) / atlas.image.height;
        glyph.geometry.hasTextureCoordinates = true;
        // Image rows run downwards; model coordinates run upwards.
        glyph.geometry.vertices = {{0,0,0,u0,v1}, {1,0,0,u1,v1}, {1,1,0,u1,v0}, {0,1,0,u0,v0}};
    }
    return atlas;
}
