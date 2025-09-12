#ifdef _WIN32
#include <windows.h>
#include <GL/gl.h>
#elif __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include <cmath>
#include "TerrainRenderer.h"
#include "RenderData.h"
#include "../App.h"
#include "../Logger.h"

TerrainRenderer::TerrainRenderer() : BaseRenderer(),
                                     displayListInitialized(false),
                                     lastHeightX(0),
                                     lastHeightZ(0),
                                     currentTerrainData(nullptr)
{
    InitializeColorPalette();
}

bool TerrainRenderer::Initialize()
{
    if (!BaseRenderer::Initialize())
    {
        return false;
    }

    // Initialize any terrain-specific resources here
    displayListInitialized = false;

    return true;
}

void TerrainRenderer::Setup3DRenderState()
{
    // Call base class setup first
    BaseRenderer::Setup3DRenderState();
    
    // Color material is now handled at the pipeline level in SetupLighting()
    // so we don't need to set it here
    
    // Ensure lighting is enabled (should already be enabled by RenderingPipeline)
    glEnable(GL_LIGHTING);
    
    // Set texture environment to modulate texture with material color
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    
    CheckGLError("TerrainRenderer::Setup3DRenderState");
}

void TerrainRenderer::Cleanup()
{
    displayListInitialized = false;
    BaseRenderer::Cleanup();
    Logger::Get().Write("TerrainRenderer cleaned up \n");
}

void TerrainRenderer::RenderTerrain(const TerrainRenderData &terrainData)
{
    if (!IsReady())
    {
        return;
    }

    // Save OpenGL state
    SetupRenderState();

    try
    {
        SetupTerrainColors(terrainData);

        glEnable(GL_DEPTH_TEST);
        // Re-enable texturing now that we've fixed the lighting issue
        glEnable(GL_TEXTURE_2D);

        // Render terrain components in order
        RenderFloatingElements(terrainData);
        RenderTerrainSurface(terrainData);
        RenderTerrainWalls(terrainData);
        RenderBoundaryWalls(terrainData);
        RenderWaterEffects(terrainData);
    }
    catch (...)
    {
        Logger::Get().Write("Exception occurred during terrain rendering");
    }

    // Restore OpenGL state
    CleanupRenderState();
}

void TerrainRenderer::RenderFloatingElements(const TerrainRenderData &terrain)
{
    glFrontFace(GL_CCW);
    glColor3f(terrain.colors.blockColor.x, terrain.colors.blockColor.y, terrain.colors.blockColor.z);

    // Use surface texture for floating elements to match the terrain surface
    BindWallTexture(terrain.levelNumber);

    for (int q = 0; q < terrain.sizeX; q++)
    {
        for (int w = 0; w < terrain.sizeZ; w++)
        {
            if (terrain.floatMap[q][w] != 0)
            {
                glPushMatrix();
                glTranslatef(q + 0.5f, terrain.floatMap[q][w] - 0.5f, w + 0.5f);

                // Use the cube display list from GraphicsTask
                if (App::GetSingleton().graphicsTask)
                {
                    App::GetSingleton().graphicsTask->cubelist1.Call(0);
                }

                glPopMatrix();
            }
        }
    }

    glFrontFace(GL_CW);
}

void TerrainRenderer::RenderTerrainSurface(const TerrainRenderData &terrain)
{
    int sx = terrain.sizeX - 1;
    int sz = terrain.sizeZ - 1;

    glNormal3f(0, 1, 0);

    // Render horizontal terrain strips
    for (int jx = 0; jx < sx; jx++)
    {
        float currentY = terrain.heightMap[jx][0];
        int stripLength = 0;

        for (int jz = 0; jz < sz; jz++)
        {
            stripLength++;

            if (currentY != terrain.heightMap[jx][jz] || jz == sz - 1)
            {
                // Bind surface-specific texture (different from walls)
                BindSurfaceTexture(terrain.levelNumber, (int)currentY);

                // Render terrain strip
                RenderTerrainQuad(jx, jz, currentY, stripLength, jz == sz - 1);
                stripLength = 0;
            }

            currentY = terrain.heightMap[jx][jz];
        }
    }
}

void TerrainRenderer::RenderTerrainWalls(const TerrainRenderData &terrain)
{
    int sx = terrain.sizeX - 1;
    int sz = terrain.sizeZ - 1;

    // Bind wall-specific texture (different from surface)
    BindWallTexture(terrain.levelNumber);

    // Render walls in X direction (front faces)
    for (int ix = 0; ix <= sx; ix++)
    {
        int lastY = 0;
        for (int iz = 0; iz <= sz; iz++)
        {
            if (terrain.heightMap[ix][iz] != lastY && iz != sz && ix != 0)
            {
                RenderWallQuad(ix, iz, lastY, terrain.heightMap[ix][iz], 0);
            }
            lastY = terrain.heightMap[ix][iz];
        }
    }

    // Render walls in X direction (back faces)
    for (int ix = sx; ix > 0; ix--)
    {
        int lastY = 0;
        for (int iz = sz - 1; iz > 0; iz--)
        {
            if (terrain.heightMap[ix][iz] != lastY && ix != sx)
            {
                RenderWallQuad(ix, iz, lastY, terrain.heightMap[ix][iz], 1);
            }
            lastY = terrain.heightMap[ix][iz];
        }
    }

    // Render walls in Z direction (left faces)
    for (int iz = 0; iz < sz; iz++)
    {
        int lastY = 0;
        for (int ix = 0; ix <= sx; ix++)
        {
            if (terrain.heightMap[ix][iz] != lastY && ix != sx && iz != 0)
            {
                RenderWallQuad(ix, iz, lastY, terrain.heightMap[ix][iz], 2);
            }
            lastY = terrain.heightMap[ix][iz];
        }
    }

    // Render walls in Z direction (right faces)
    for (int iz = sz; iz > 0; iz--)
    {
        int lastY = 0;
        for (int ix = sx; ix >= 0; ix--)
        {
            if (terrain.heightMap[ix][iz] != lastY && ix != 0)
            {
                RenderWallQuad(ix, iz, lastY, terrain.heightMap[ix][iz], 3);
            }
            lastY = terrain.heightMap[ix][iz];
        }
    }
}

void TerrainRenderer::RenderBoundaryWalls(const TerrainRenderData &terrain)
{
    int sx = terrain.sizeX - 1;
    int sz = terrain.sizeZ - 1;

    glColor3f(terrain.colors.defaultColor.x, terrain.colors.defaultColor.y, terrain.colors.defaultColor.z);

    // Special handling for level 48 (underground level)
    if (terrain.levelNumber == 48)
    {
        glTranslatef(0, -30, 0);
        glFrontFace(GL_CCW);
    }

    // Top ceiling (for non-open levels)
    if (terrain.levelNumber != 48)
    {
        glBegin(GL_QUADS);
        glTexCoord2f(sx - 1, sz);
        glVertex3f(1.0f, 26.0f, 1.0f);
        glTexCoord2f(0, sz);
        glVertex3f(1.0f, 26.0f, sz);
        glTexCoord2f(0, 0);
        glVertex3f(sz, 26.0f, sz);
        glTexCoord2f(sx - 1, 0);
        glVertex3f(sz, 26.0f, 1.0f);
        glEnd();
    }

    // Boundary walls (4 walls around the level)
    // Front wall
    glBegin(GL_QUADS);
    glColor3f(terrain.colors.defaultColor.x, terrain.colors.defaultColor.y, terrain.colors.defaultColor.z);
    glTexCoord2f(0, 30);
    glVertex3f(1.0f, 30.0f, 1.0f);
    glTexCoord2f(sx - 1, 30);
    glVertex3f(sx, 30.0f, 1.0f);
    glColor3f(terrain.colors.blockColor.x, terrain.colors.blockColor.y, terrain.colors.blockColor.z);
    glTexCoord2f(sx - 1, 0);
    glVertex3f(sx, 0.0f, 1.0f);
    glTexCoord2f(0, 0);
    glVertex3f(1.0f, 0.0f, 1.0f);
    glEnd();

    // Back wall
    glBegin(GL_QUADS);
    glColor3f(terrain.colors.defaultColor.x, terrain.colors.defaultColor.y, terrain.colors.defaultColor.z);
    glTexCoord2f(sx - 1, 30);
    glVertex3f(sx, 30.0f, sz);
    glTexCoord2f(0, 30);
    glVertex3f(1.0f, 30.0f, sz);
    glColor3f(terrain.colors.blockColor.x, terrain.colors.blockColor.y, terrain.colors.blockColor.z);
    glTexCoord2f(0, 0);
    glVertex3f(1.0f, 0.0f, sz);
    glTexCoord2f(sx - 1, 0);
    glVertex3f(sx, 0.0f, sz);
    glEnd();

    // Right wall
    glBegin(GL_QUADS);
    glColor3f(terrain.colors.defaultColor.x, terrain.colors.defaultColor.y, terrain.colors.defaultColor.z);
    glTexCoord2f(0, 30);
    glVertex3f(sz, 30.0f, 1.0f);
    glTexCoord2f(sx - 1, 30);
    glVertex3f(sx, 30.0f, sz);
    glColor3f(terrain.colors.blockColor.x, terrain.colors.blockColor.y, terrain.colors.blockColor.z);
    glTexCoord2f(sx - 1, 0);
    glVertex3f(sx, 0.0f, sz);
    glTexCoord2f(0, 0);
    glVertex3f(sz, 0.0f, 1.0f);
    glEnd();

    // Left wall
    glBegin(GL_QUADS);
    glColor3f(terrain.colors.defaultColor.x, terrain.colors.defaultColor.y, terrain.colors.defaultColor.z);
    glTexCoord2f(0, 30);
    glVertex3f(1.0f, 30.0f, sz);
    glTexCoord2f(sx - 1, 30);
    glVertex3f(1.0f, 30.0f, 1.0f);
    glColor3f(terrain.colors.blockColor.x, terrain.colors.blockColor.y, terrain.colors.blockColor.z);
    glTexCoord2f(sx - 1, 0);
    glVertex3f(1.0f, 0.0f, 1.0f);
    glTexCoord2f(0, 0);
    glVertex3f(1.0f, 0.0f, sz);
    glEnd();

    // Restore translation for level 48
    if (terrain.levelNumber == 48)
    {
        glTranslatef(0, 30, 0);
    }
}

void TerrainRenderer::RenderWaterEffects(const TerrainRenderData &terrain)
{
    int sx = terrain.sizeX - 1;
    int sz = terrain.sizeZ - 1;

    // Enable blending for water effects
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);

    // Bind water texture
    if (App::GetSingleton().graphicsTask)
    {
        glBindTexture(GL_TEXTURE_2D,
                      App::GetSingleton().graphicsTask->textureHandler.GetTextureArray()[TEXTURE_BLEND]);
    }

    glColor4f(terrain.colors.defaultColor.x, terrain.colors.defaultColor.y, terrain.colors.defaultColor.z, 0.5f);

    // Render water effects for negative height areas
    for (int direction = 0; direction < 4; direction++)
    {
        int lastY = 0;

        if (direction == 0)
        {
            // X direction forward
            for (int ix = 0; ix <= sx; ix++)
            {
                for (int iz = 0; iz <= sz; iz++)
                {
                    if (terrain.heightMap[ix][iz] != lastY && iz != sz)
                    {
                        if (terrain.heightMap[ix][iz] < 0)
                        {
                            RenderBlendQuad(ix, iz, lastY, direction);
                        }
                    }
                    lastY = terrain.heightMap[ix][iz];
                }
            }
        }
        else if (direction == 1)
        {
            // X direction backward
            for (int ix = sx; ix > 0; ix--)
            {
                for (int iz = sz - 1; iz > 0; iz--)
                {
                    if (terrain.heightMap[ix][iz] != lastY)
                    {
                        if (terrain.heightMap[ix][iz] < 0)
                        {
                            RenderBlendQuad(ix, iz, lastY, direction);
                        }
                    }
                    lastY = terrain.heightMap[ix][iz];
                }
            }
        }
        else if (direction == 2)
        {
            // Z direction forward
            for (int iz = 0; iz < sz; iz++)
            {
                for (int ix = 0; ix <= sx; ix++)
                {
                    if (terrain.heightMap[ix][iz] != lastY && ix != sx)
                    {
                        if (terrain.heightMap[ix][iz] < 0)
                        {
                            RenderBlendQuad(ix, iz, lastY, direction);
                        }
                    }
                    lastY = terrain.heightMap[ix][iz];
                }
            }
        }
        else if (direction == 3)
        {
            // Z direction backward
            for (int iz = sz; iz > 0; iz--)
            {
                for (int ix = sx; ix >= 0; ix--)
                {
                    if (terrain.heightMap[ix][iz] != lastY && ix != 0)
                    {
                        if (terrain.heightMap[ix][iz] < 0)
                        {
                            RenderBlendQuad(ix, iz, lastY, direction);
                        }
                    }
                    lastY = terrain.heightMap[ix][iz];
                }
            }
        }
    }

    // Restore OpenGL state
    glColor3f(terrain.colors.defaultColor.x, terrain.colors.defaultColor.y, terrain.colors.defaultColor.z);
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
}

void TerrainRenderer::SetupTerrainColors(const TerrainRenderData &terrain)
{
    // Use metadata colors instead of hardcoded palette
    // This replaces the OpenGL color calls that were previously in LevelHandler::DrawTerrain_OLD()
    
    // Store the current terrain data for use in rendering methods
    currentTerrainData = &terrain;
    
    // Set default color as the base terrain color
    const auto &colors = terrain.colors;
    
    // Only log when color changes to avoid spam (using epsilon for float comparison)
    static Vector3 lastLoggedColor(-2.0f, -2.0f, -2.0f);  // Use invalid color as initial value
    static bool hasLogged = false;
    
    const float epsilon = 0.001f;
    bool colorChanged = !hasLogged ||
                       std::abs(colors.defaultColor.x - lastLoggedColor.x) > epsilon || 
                       std::abs(colors.defaultColor.y - lastLoggedColor.y) > epsilon || 
                       std::abs(colors.defaultColor.z - lastLoggedColor.z) > epsilon;
    
    if (colorChanged) {
        // Convert internal level number to logical level number for clearer logging
        int logicalLevel = (terrain.levelNumber >= 48 && terrain.levelNumber <= 57) ? 
                          terrain.levelNumber - 48 : 
                          (terrain.levelNumber >= 65) ? terrain.levelNumber - 65 + 10 : 0;
        Logger::Get().Write("TerrainRenderer: Level %d (internal %d) - Setting terrain color to (%.2f, %.2f, %.2f)\n", 
            logicalLevel, terrain.levelNumber,
            colors.defaultColor.x, colors.defaultColor.y, colors.defaultColor.z);
        lastLoggedColor = colors.defaultColor;
        hasLogged = true;
    }
    
    // Set the OpenGL color
    glColor3f(colors.defaultColor.x, colors.defaultColor.y, colors.defaultColor.z);
    
    // Debug: Verify the color was actually set and check for any color matrix transformations
    GLfloat setColor[4];
    glGetFloatv(GL_CURRENT_COLOR, setColor);
    if (colorChanged) {
        Logger::Get().Write("TerrainRenderer: OpenGL color after glColor3f: (%.2f, %.2f, %.2f, %.2f)\n", 
                           setColor[0], setColor[1], setColor[2], setColor[3]);
        
        // Check if any lighting is affecting colors
        GLboolean lightingEnabled = glIsEnabled(GL_LIGHTING);
        GLboolean colorMaterialEnabled = glIsEnabled(GL_COLOR_MATERIAL);
        Logger::Get().Write("TerrainRenderer: GL_LIGHTING=%d, GL_COLOR_MATERIAL=%d\n", 
                           lightingEnabled, colorMaterialEnabled);
    }
}

void TerrainRenderer::SetPrimaryColor()
{
    if (currentTerrainData) {
        const auto &color = currentTerrainData->colors.defaultColor;
        glColor3f(color.x, color.y, color.z);
    }
}

void TerrainRenderer::SetSecondaryColor()
{
    if (currentTerrainData) {
        const auto &color = currentTerrainData->colors.blockColor;
        glColor3f(color.x, color.y, color.z);
    }
}

void TerrainRenderer::SetBlockColor()
{
    if (currentTerrainData) {
        const auto &color = currentTerrainData->colors.blockColor;
        glColor3f(color.x, color.y, color.z);
    }
}

void TerrainRenderer::BindSurfaceTexture(int levelNumber, int currentHeight)
{
    if (!App::GetSingleton().graphicsTask)
    {
        return;
    }

    auto *textureArray = App::GetSingleton().graphicsTask->textureHandler.GetTextureArray();

    // Select surface texture based on level
    // For title screen (level 0 and some others), use checker pattern for top surface
    if (levelNumber == 0 || levelNumber == 50 || levelNumber == 56 || levelNumber == 57 || levelNumber == 58)
    {
        if(currentHeight == 0)
        {
            glBindTexture(GL_TEXTURE_2D, textureArray[TEXTURE_BLACK]);
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D, textureArray[TEXTURE_CHECKER]);
        }
    }
    else if (levelNumber == 48 || levelNumber == 70 || levelNumber == 69)
    {
        if(currentHeight == 0)
        {
            glBindTexture(GL_TEXTURE_2D, textureArray[TEXTURE_BLACK]);
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D, textureArray[TEXTURE_WHITE]);
        }
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, textureArray[TEXTURE_BLACK]);
    }
}

void TerrainRenderer::BindWallTexture(int levelNumber)
{
    if (!App::GetSingleton().graphicsTask)
    {
        return;
    }

    auto *textureArray = App::GetSingleton().graphicsTask->textureHandler.GetTextureArray();
    
    glBindTexture(GL_TEXTURE_2D, textureArray[TEXTURE_BLACK]);

}

void TerrainRenderer::RenderTerrainQuad(int x, int z, int height, int strips, bool isLast)
{
    if (height >= 25)
        return; // Skip high terrain

    // Set color based on height - use metadata colors instead of palette
    if (height == 0)
    {
        if (currentTerrainData) {
            glColor3f(currentTerrainData->colors.blockColor.x, 
                     currentTerrainData->colors.blockColor.y, 
                     currentTerrainData->colors.blockColor.z);
        }
    }
    else
    {
        if (currentTerrainData) {
            glColor3f(currentTerrainData->colors.defaultColor.x, 
                     currentTerrainData->colors.defaultColor.y, 
                     currentTerrainData->colors.defaultColor.z);
        }
    }

    glBegin(GL_QUADS);

    if (isLast)
    {
        glTexCoord2i(strips + 1, 1);
        glVertex3i(x, height, z - strips);
        glTexCoord2i(strips + 1, 0);
        glVertex3i(x + 1, height, z - strips);
        glTexCoord2i(0, 0);
        glVertex3i(x + 1, height, z + 1);
        glTexCoord2i(0, 1);
        glVertex3i(x, height, z + 1);
    }
    else
    {
        if (height >= 0)
        {
            glTexCoord2i(strips, 1);
            glVertex3i(x, height, z - strips);
            glTexCoord2i(strips, 0);
            glVertex3i(x + 1, height, z - strips);
            glTexCoord2i(0, 0);
            glVertex3i(x + 1, height, z);
            glTexCoord2i(0, 1);
            glVertex3i(x, height, z);
        }
    }

    glEnd();
}

void TerrainRenderer::RenderWallQuad(int ix, int iz, int lastY, int currentY, int direction)
{
    for (int iy = lastY; iy <= currentY; iy++)
    {
        if (iy == currentY)
        {
            glBegin(GL_QUADS);

            // Set colors based on height - use metadata colors
            if (lastY < 0)
            {
                if (currentTerrainData) {
                    glColor3f(currentTerrainData->colors.blockColor.x, 
                             currentTerrainData->colors.blockColor.y, 
                             currentTerrainData->colors.blockColor.z);
                }
            }
            else
            {
                if (currentTerrainData) {
                    glColor3f(currentTerrainData->colors.defaultColor.x, 
                             currentTerrainData->colors.defaultColor.y, 
                             currentTerrainData->colors.defaultColor.z);
                }
            }

            // Render quad based on direction
            switch (direction)
            {
            case 0: // X direction front
                glTexCoord2f(0, 0);
                glVertex3f(ix, lastY, iz);
                glTexCoord2f(1, 0);
                glVertex3f(ix + 1, lastY, iz);
                glTexCoord2f(1, iy - lastY);
                glVertex3f(ix + 1, iy, iz);
                glTexCoord2f(0, iy - lastY);
                glVertex3f(ix, iy, iz);
                break;

            case 1: // X direction back
                glTexCoord2i(0, 0);
                glVertex3i(ix, iy, iz + 1);
                glTexCoord2i(1, 0);
                glVertex3i(ix + 1, iy, iz + 1);
                glTexCoord2i(1, iy - lastY);
                glVertex3i(ix + 1, lastY, iz + 1);
                glTexCoord2i(0, iy - lastY);
                glVertex3i(ix, lastY, iz + 1);
                break;

            case 2: // Z direction left
                glTexCoord2i(0, 0);
                glVertex3i(ix, iy, iz);
                glTexCoord2i(1, 0);
                glVertex3i(ix, iy, iz + 1);
                glTexCoord2i(1, iy - lastY);
                glVertex3i(ix, lastY, iz + 1);
                glTexCoord2i(0, iy - lastY);
                glVertex3i(ix, lastY, iz);
                break;

            case 3: // Z direction right
                glTexCoord2f(0, 0);
                glVertex3f(ix + 1, lastY, iz);
                glTexCoord2f(1, 0);
                glVertex3f(ix + 1, lastY, iz + 1);
                glTexCoord2f(1, iy - lastY);
                glVertex3f(ix + 1, iy, iz + 1);
                glTexCoord2f(0, iy - lastY);
                glVertex3f(ix + 1, iy, iz);
                break;
            }

            // Restore main color
            if (currentTerrainData) {
                glColor3f(currentTerrainData->colors.defaultColor.x, 
                         currentTerrainData->colors.defaultColor.y, 
                         currentTerrainData->colors.defaultColor.z);
            }

            glEnd();
        }
    }
}

void TerrainRenderer::RenderBlendQuad(int ix, int iz, int height, int direction)
{
    glBegin(GL_QUADS);

    switch (direction)
    {
    case 0: // X direction forward
        glTexCoord2i(0, 1);
        glVertex3f(ix, height + 2, iz);
        glTexCoord2i(1, 1);
        glVertex3f(ix + 1, height + 2, iz);
        glTexCoord2i(1, 0);
        glVertex3f(ix + 1, height, iz);
        glTexCoord2i(0, 0);
        glVertex3f(ix, height, iz);
        break;

    case 1: // X direction backward
        glTexCoord2i(0, 1);
        glVertex3f(ix, height + 2, iz + 1);
        glTexCoord2i(1, 1);
        glVertex3f(ix + 1, height + 2, iz + 1);
        glTexCoord2i(1, 0);
        glVertex3f(ix + 1, height, iz + 1);
        glTexCoord2i(0, 0);
        glVertex3f(ix, height, iz + 1);
        break;

    case 2: // Z direction forward
        glTexCoord2i(0, 1);
        glVertex3f(ix, height + 2, iz);
        glTexCoord2i(1, 1);
        glVertex3f(ix, height + 2, iz + 1);
        glTexCoord2i(1, 0);
        glVertex3f(ix, height, iz + 1);
        glTexCoord2i(0, 0);
        glVertex3f(ix, height, iz);
        break;

    case 3: // Z direction backward
        glTexCoord2i(0, 1);
        glVertex3f(ix + 1, height + 2, iz);
        glTexCoord2i(1, 1);
        glVertex3f(ix + 1, height + 2, iz + 1);
        glTexCoord2i(1, 0);
        glVertex3f(ix + 1, height, iz + 1);
        glTexCoord2i(0, 0);
        glVertex3f(ix + 1, height, iz);
        break;
    }

    glEnd();
}

void TerrainRenderer::InitializeColorPalette()
{
    // Initialize the 32-color palette used for terrain rendering
    float colors[COLOR_PALETTE_SIZE][4] = {
        {1.0, 1.0, 1.0, 1.0}, {0.0, 1.0, 0.0, 1.0}, {1.0, 0.0, 0.0, 1.0}, {0.0, 0.0, 1.0, 1.0}, {1.0, 0.0, 1.0, 1.0}, {0.0, 1.0, 0.0, 1.0}, {1.0, 1.0, 0.0, 1.0}, {0.0, 0.0, 1.0, 1.0}, {0.8, 1.0, 0.5, 1.0}, {1.0, 1.0, 1.0, 1.0}, {0.5, 0.9, 0.0, 1.0}, {0.5, 0.5, 0.5, 1.0}, {0.0, 1.0, 0.5, 1.0}, {0.3, 0.7, 0.7, 1.0}, {0.5, 1.0, 0.0, 1.0}, {0.8, 0.7, 0.0, 1.0}, {1.0, 1.0, 0.0, 1.0}, {0.0, 1.0, 0.0, 1.0}, {1.0, 0.0, 0.0, 1.0}, {0.0, 0.0, 1.0, 1.0}, {1.0, 0.0, 1.0, 1.0}, {0.0, 1.0, 1.0, 1.0}, {1.0, 0.0, 0.5, 1.0}, {0.0, 0.0, 1.0, 1.0}, {0.8, 1.0, 0.5, 1.0}, {1.0, 1.0, 1.0, 1.0}, {0.5, 0.9, 0.0, 1.0}, {0.0, 1.0, 0.2, 1.0}, {0.0, 1.0, 0.5, 1.0}, {0.3, 0.7, 0.7, 1.0}, {0.5, 1.0, 0.0, 1.0}, {0.0, 0.0, 0.0, 1.0}};

    for (int i = 0; i < COLOR_PALETTE_SIZE; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            colorPalette[i][j] = colors[i][j];
        }
    }
}
