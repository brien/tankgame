#ifndef TextRenderer_h
#define TextRenderer_h


#include "DisplayList.h"

class TextRenderer
{
public:
    static DisplayList displayList;
    static const float LETTER_WIDTH;
    static const float LETTER_HEIGHT;
    static const int COLOR_NUM;

    static void init();

    static void close();

    static float getWidth(int n, float s);

    static float getHeight(float s);

    static enum Direction {
        TO_RIGHT, TO_DOWN, TO_LEFT, TO_UP,
    };

    static int convertCharToInt(char c);

    static void drawString(char str[], float lx, float y, float s,
        int d = Direction::TO_RIGHT, int cl = 0,
        bool rev = false, float od = 0);

    static void drawNum(int num, float lx, float y, float s,
        int d = Direction::TO_RIGHT, int cl = 0, int dg = 0);

    static void drawNumSign(int num, float lx, float ly, float s, int cl);

    static void drawTime(int time, float lx, float y, float s, int cl = 0);

private:
    static const float COLOR_RGB[][3];
    static const int LETTER_NUM;
    static const int DISPLAY_LIST_NUM;

    static void drawLetter(int n, float x, float y, float s, float d, int c);

    static void drawLetterRev(int n, float x, float y, float s, float d, int c);

    static void drawLetter(int idx, int c);

    static void drawBox(float x, float y, float width, float height, float deg,
        float r, float g, float b);

    static void drawBoxLine(float x, float y, float width, float height, float deg);
    static void drawBoxPoly(float x, float y, float width, float height, float deg);
    static void drawBoxPart(float width, float height);

    static float spData[][16][5];
};

#endif /* TextRenderer_h */
