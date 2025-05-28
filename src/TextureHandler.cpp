#ifdef _WIN32
// If building in windows:
#pragma warning(disable : 4996)
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#elif __APPLE__
// If building on macOS:
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
// If building on Linux:
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "TextureHandler.h"
#include <stdlib.h>
#include <cstdint>
#include <iostream>

#include "App.h"

typedef uint16_t WORD; // WORD is typically a 16-bit unsigned integer

TextureHandler::TextureHandler()
{
}

TextureHandler::~TextureHandler()
{
}

void TextureHandler::LoadTextures()
{
    TGA_Texture(textureArray, "texture/cube1.tga", TEXTURE_WHITE_CUBE, true);
    TGA_Texture(textureArray, "texture/cube2.tga", TEXTURE_BLACK_CUBE, true);
    TGA_Texture(textureArray, "texture/trail.tga", TEXTURE_EXIT, false);
    TGA_Texture(textureArray, "texture/bang.tga", TEXTURE_BANG, true);
    TGA_Texture(textureArray, "texture/x.tga", TEXTURE_X, true);
    TGA_Texture(textureArray, "texture/cube12.tga", TEXTURE_CHECKER, true);
    TGA_Texture(textureArray, "texture/heart.tga", TEXTURE_HEART, true);
    TGA_Texture(textureArray, "texture/p_itemstar.tga", TEXTURE_DIAMOND, true);
    TGA_Texture(textureArray, "texture/p.tga", TEXTURE_P, true);
    TGA_Texture(textureArray, "texture/star.tga", TEXTURE_STAR, true);
    TGA_Texture(textureArray, "texture/ring.tga", TEXTURE_RING, true);
    TGA_Texture(textureArray, "texture/long.tga", TEXTURE_LONGSHOT, true);
    TGA_Texture(textureArray, "texture/bank.tga", TEXTURE_BANKSHOT, true);
    TGA_Texture(textureArray, "texture/multi.tga", TEXTURE_MULTISHOT, true);
    TGA_Texture(textureArray, "texture/score.tga", TEXTURE_SCORE, true);
    TGA_Texture(textureArray, "texture/enemy.tga", TEXTURE_ENEMY, true);

    TGA_Texture(textureArray, "texture/0.tga", TEXTURE_ZERO, true);
    TGA_Texture(textureArray, "texture/1.tga", TEXTURE_ONE, true);
    TGA_Texture(textureArray, "texture/2.tga", TEXTURE_TWO, true);
    TGA_Texture(textureArray, "texture/3.tga", TEXTURE_THREE, true);
    TGA_Texture(textureArray, "texture/4.tga", TEXTURE_FOUR, true);
    TGA_Texture(textureArray, "texture/5.tga", TEXTURE_FIVE, true);
    TGA_Texture(textureArray, "texture/6.tga", TEXTURE_SIX, true);
    TGA_Texture(textureArray, "texture/7.tga", TEXTURE_SEVEN, true);
    TGA_Texture(textureArray, "texture/8.tga", TEXTURE_EIGHT, true);
    TGA_Texture(textureArray, "texture/9.tga", TEXTURE_NINE, true);
}

void TextureHandler::TGA_Texture(unsigned int textureArray[], const char *strFileName, int ID, bool wrap)
{
    if (!strFileName)
    {
        return;
    }

    tImageTGA *pBitMap = Load_TGA(strFileName);

    if (pBitMap == NULL)
        return;

    glGenTextures(1, &textureArray[ID]);
    glBindTexture(GL_TEXTURE_2D, textureArray[ID]);
    int textureType = GL_RGB;
    if (pBitMap->channels == 4)
        textureType = GL_RGBA;
    gluBuild2DMipmaps(GL_TEXTURE_2D, pBitMap->channels, pBitMap->size_x, pBitMap->size_y, textureType, GL_UNSIGNED_BYTE, pBitMap->data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    if (!wrap)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    if (pBitMap)
    {
        if (pBitMap->data)
        {
            free(pBitMap->data);
        }
        free(pBitMap);
    }
}

tImageTGA *TextureHandler::Load_TGA(const char *strfilename)
{
    tImageTGA *pImgData = NULL;
    FILE *pFile = NULL;

    WORD width = 0;
    WORD height = 0;
    unsigned char length = 0;
    unsigned char imgType = 0;
    unsigned char bits = 0;
    int channels = 0;
    int stride = 0;
    int i = 0;

    if ((pFile = fopen(strfilename, "rb")) == NULL)
    {
        std::cerr << "GraphicsTask:Load_TGA: Failed to open " << strfilename << std::endl;
        // GuiTask::RenderSpacedBitmapString(30,30,10,(void *)GuiTask::font,"ERROR");
        return NULL;
    }

    pImgData = (tImageTGA *)malloc(sizeof(tImageTGA));

    fread(&length, sizeof(unsigned char), 1, pFile);

    fseek(pFile, 1, SEEK_CUR);

    fread(&imgType, sizeof(unsigned char), 1, pFile);

    fseek(pFile, 9, SEEK_CUR);

    fread(&width, sizeof(WORD), 1, pFile);
    fread(&height, sizeof(WORD), 1, pFile);
    fread(&bits, sizeof(unsigned char), 1, pFile);

    fseek(pFile, length + 1, SEEK_CUR);

    if (imgType != TGA_RLE)
    {
        // Check for 24 or 32 Bit
        if (bits == 24 || bits == 32)
        {

            channels = bits / 8;
            stride = channels * width;
            pImgData->data = new unsigned char[stride * height];

            for (int y = 0; y < height; y++)
            {
                unsigned char *pLine = &(pImgData->data[stride * y]);

                fread(pLine, stride, 1, pFile);

                for (i = 0; i < stride; i += channels)
                {
                    int temp = pLine[i];
                    pLine[i] = pLine[i + 2];
                    pLine[i + 2] = temp;
                }
            }
        }

        // Check for 16 Bit
        else if (bits == 16)
        {
            unsigned short pixels = 0;
            int r = 0, g = 0, b = 0;

            channels = 3;
            stride = channels * width;
            pImgData->data = new unsigned char[stride * height];

            for (int i = 0; i < width * height; i++)
            {
                fread(&pixels, sizeof(unsigned short), 1, pFile);

                b = (pixels & 0x1f) << 3;
                g = ((pixels >> 5) & 0x1f) << 3;
                r = ((pixels >> 10) & 0x1f) << 3;

                pImgData->data[i * 3 + 0] = r;
                pImgData->data[i * 3 + 1] = g;
                pImgData->data[i * 3 + 2] = b;
            }
        }
        else
        {
            return NULL;
        }
    }
    else
    {
        unsigned char rleID = 0;
        int colorsRead = 0;
        channels = bits / 8;
        stride = channels * width;

        pImgData->data = new unsigned char[stride * height];
        unsigned char *pColors = new unsigned char[channels];

        while (i < width * height)
        {

            fread(&rleID, sizeof(unsigned char), 1, pFile);

            if (rleID < 128)
            {
                rleID++;

                while (rleID)
                {
                    fread(pColors, sizeof(unsigned char) * channels, 1, pFile);

                    pImgData->data[colorsRead + 0] = pColors[2];
                    pImgData->data[colorsRead + 1] = pColors[1];
                    pImgData->data[colorsRead + 2] = pColors[0];

                    if (bits == 32)
                        pImgData->data[colorsRead + 3] = pColors[3];

                    i++;
                    rleID--;
                    colorsRead += channels;
                }
            }
            else
            {
                rleID -= 127;

                fread(pColors, sizeof(unsigned char) * channels, 1, pFile);

                while (rleID)
                {
                    pImgData->data[colorsRead + 0] = pColors[2];
                    pImgData->data[colorsRead + 1] = pColors[1];
                    pImgData->data[colorsRead + 2] = pColors[0];

                    if (bits == 32)
                        pImgData->data[colorsRead + 3] = pColors[3];

                    i++;
                    rleID--;
                    colorsRead += channels;
                }
            }
        }
    }

    fclose(pFile);

    pImgData->channels = channels;
    pImgData->size_x = width;
    pImgData->size_y = height;

    return pImgData;
}
