#ifndef TEXTUREHANDLER_H_INCLUDED
#define TEXTUREHANDLER_H_INCLUDED

#define TGA_RGB		2
#define TGA_A		3
#define TGA_RLE		10

enum TextureNames
{
    TEXTURE_ZERO,
    TEXTURE_ONE,
    TEXTURE_TWO,
    TEXTURE_THREE,
    TEXTURE_FOUR,
    TEXTURE_FIVE,
    TEXTURE_SIX,
    TEXTURE_SEVEN,
    TEXTURE_EIGHT,
    TEXTURE_NINE,
    TEXTURE_WHITE_CUBE,
    TEXTURE_BLACK_CUBE,
    TEXTURE_EXIT,
    TEXTURE_BANG,
    TEXTURE_X,
    TEXTURE_CHECKER,
    TEXTURE_HEART,
    TEXTURE_DIAMOND,
    TEXTURE_P,
    TEXTURE_STAR,
    TEXTURE_RING,
    TEXTURE_LONGSHOT,
    TEXTURE_BANKSHOT,
    TEXTURE_MULTISHOT,
    TEXTURE_SCORE,
    TEXTURE_ENEMY,

    TEXTURE_NAMES_COUNT
};

struct tImageTGA
{
    int channels;
    int size_x;
    int size_y;
    unsigned char* data;
};


class TextureHandler
{
public:
    TextureHandler();
    ~TextureHandler();

    void LoadTextures();

    unsigned int* GetTextureArray() { return textureArray; }

private:
    unsigned int textureArray[32];

    void TGA_Texture(unsigned int textureArray[], const char* strFileName, int ID, bool wrap);
    tImageTGA* Load_TGA(const char* strfilename);
};

#endif
