#ifdef _WIN32
// If building in windows:
#pragma warning(disable : 4996)
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#else
//if not:
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#endif

#include "TextRenderer.h"
#include "math.h"

DisplayList TextRenderer::displayList = DisplayList(DISPLAY_LIST_NUM);

const float TextRenderer::LETTER_WIDTH = 2.1f;
const float TextRenderer::LETTER_HEIGHT = 3.0f;
const int TextRenderer::COLOR_NUM = 4;

const float TextRenderer::COLOR_RGB[][3] = { {1, 1, 1}, {0.9, 0.7, 0.5} };

const int TextRenderer::LETTER_NUM = 44;
const int TextRenderer::DISPLAY_LIST_NUM = LETTER_NUM * COLOR_NUM;

void TextRenderer::init()
{
    displayList = DisplayList(DISPLAY_LIST_NUM);
    displayList.ResetList();
    for (int j = 0; j < COLOR_NUM; j++) {
        for (int i = 0; i < LETTER_NUM; i++) {
            displayList.NewList();
            drawLetter(i, j);
            displayList.EndList();
        }
    }
}

void TextRenderer::close() {
    displayList.Close();
}

float TextRenderer::getWidth(int n, float s) {
    return n * s * LETTER_WIDTH;
}

float TextRenderer::getHeight(float s) {
    return s * LETTER_HEIGHT;
}

void TextRenderer::drawLetter(int n, float x, float y, float s, float d, int c) {
    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(s, s, s);
    glRotatef(d, 0, 0, 1);
    displayList.Call(n + c * LETTER_NUM);
    glPopMatrix();
}

void TextRenderer::drawLetterRev(int n, float x, float y, float s, float d, int c) {
    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(s, -s, s);
    glRotatef(d, 0, 0, 1);
    displayList.Call(n + c * LETTER_NUM);
    glPopMatrix();
}

static enum Direction {
    TO_RIGHT, TO_DOWN, TO_LEFT, TO_UP,
};

int TextRenderer::convertCharToInt(char c) {
    int idx;
    if (c >= '0' && c <= '9') {
        idx = c - '0';
    }
    else if (c >= 'A' && c <= 'Z') {
        idx = c - 'A' + 10;
    }
    else if (c >= 'a' && c <= 'z') {
        idx = c - 'a' + 10;
    }
    else if (c == '.' || c == ':') {
        idx = 36;
    }
    else if (c == '-') {
        idx = 38;
    }
    else if (c == '+') {
        idx = 39;
    }
    else if (c == '_') {
        idx = 37;
    }
    else if (c == '!') {
        idx = 42;
    }
    else if (c == '/') {
        idx = 43;
    }
    return idx;
}

void TextRenderer::drawString(char str[], float lx, float y, float s,
    int d, int cl,
    bool rev, float od) {
    lx += LETTER_WIDTH * s / 2;
    y += LETTER_HEIGHT * s / 2;
    float x = lx;
    int idx;
    float ld = 0;
    switch (d) {
    case Direction::TO_RIGHT:
        ld = 0;
        break;
    case Direction::TO_DOWN:
        ld = 90;
        break;
    case Direction::TO_LEFT:
        ld = 180;
        break;
    case Direction::TO_UP:
        ld = 270;
        break;
    }
    ld += od;
    for (int i = 0; i < strlen(str); i++) {

        if (str[i] != ' ') {
            idx = convertCharToInt(str[i]);
            if (rev)
                drawLetterRev(idx, x, y, s, ld, cl);
            else
                drawLetter(idx, x, y, s, ld, cl);
        }
        if (od == 0) {
            switch (d) {
            case Direction::TO_RIGHT:
                x += s * LETTER_WIDTH;
                break;
            case Direction::TO_DOWN:
                y += s * LETTER_WIDTH;
                break;
            case Direction::TO_LEFT:
                x -= s * LETTER_WIDTH;
                break;
            case Direction::TO_UP:
                y -= s * LETTER_WIDTH;
                break;
            }
        }
        else {
            x += cos(ld * PI / 180) * s * LETTER_WIDTH;
            y += sin(ld * PI / 180) * s * LETTER_WIDTH;
        }
    }
}

void TextRenderer::drawNum(int num, float lx, float y, float s,
    int d, int cl, int dg) {
    lx += LETTER_WIDTH * s / 2;
    y += LETTER_HEIGHT * s / 2;
    int n = num;
    float x = lx;
    float ld;
    switch (d) {
    case Direction::TO_RIGHT:
        ld = 0;
        break;
    case Direction::TO_DOWN:
        ld = 90;
        break;
    case Direction::TO_LEFT:
        ld = 180;
        break;
    case Direction::TO_UP:
        ld = 270;
        break;
    }
    int digit = dg;
    for (;;) {
        drawLetter(n % 10, x, y, s, ld, cl);
        switch (d) {
        case Direction::TO_RIGHT:
            x -= s * LETTER_WIDTH;
            break;
        case Direction::TO_DOWN:
            y -= s * LETTER_WIDTH;
            break;
        case Direction::TO_LEFT:
            x += s * LETTER_WIDTH;
            break;
        case Direction::TO_UP:
            y += s * LETTER_WIDTH;
            break;
        }
        n /= 10;
        digit--;
        if (n <= 0 && digit <= 0)
            break;
    }
}

void TextRenderer::drawNumSign(int num, float lx, float ly, float s, int cl) {
    float dg;
    if (num < 100)
        dg = 2;
    else if (num < 1000)
        dg = 3;
    else if (num < 10000)
        dg = 4;
    else
        dg = 5;
    float x = lx + LETTER_WIDTH * s * dg / 2;
    float y = ly + LETTER_HEIGHT * s / 2;
    int n = num;
    for (;;) {
        drawLetterRev(n % 10, x, y, s, 0, cl);
        x -= s * LETTER_WIDTH;
        n /= 10;
        if (n <= 0)
            break;
    }
}

void TextRenderer::drawTime(int time, float lx, float y, float s, int cl) {
    int n = time;
    if (n < 0)
        n = 0;
    float x = lx;
    for (int i = 0; i < 7; i++) {
        if (i != 4) {
            drawLetter(n % 10, x, y, s, Direction::TO_RIGHT, cl);
            n /= 10;
        }
        else {
            drawLetter(n % 6, x, y, s, Direction::TO_RIGHT, cl);
            n /= 6;
        }
        if ((i & 1) == 1 || i == 0) {
            switch (i) {
            case 3:
                drawLetter(41, x + s * 1.16f, y, s, Direction::TO_RIGHT, cl);
                break;
            case 5:
                drawLetter(40, x + s * 1.16f, y, s, Direction::TO_RIGHT, cl);
                break;
            default:
                break;
            }
            x -= s * LETTER_WIDTH;
        }
        else {
            x -= s * LETTER_WIDTH * 1.3f;
        }
        if (n <= 0)
            break;
    }
}

void TextRenderer::drawLetter(int idx, int c) {
    float x, y, length, size, t;
    int deg;
    for (int i = 0;; i++) {
        //deg = (int) spData[idx][i][4];
        deg = (int) spData[4][idx][i];
        if (deg > 99990) break;
        x = -spData[0][idx][i];
        y = -spData[1][idx][i];
        size = spData[2][idx][i];
        length = spData[3][idx][i];
        y *= 0.9;
        size *= 1.4;
        length *= 1.05;
        x = -x;
        y = y;
        deg %= 180;
        if (c == 2)
            drawBoxLine(x, y, size, length, deg);
        else if (c == 3)
            drawBoxPoly(x, y, size, length, deg);
        else
            drawBox(x, y, size, length, deg,
                COLOR_RGB[c][0], COLOR_RGB[c][1], COLOR_RGB[c][2]);
    }
}

void TextRenderer::drawBox(float x, float y, float width, float height, float deg,
    float r, float g, float b) {
    glPushMatrix();
    glTranslatef(x - width / 2, y - height / 2, 0);
    glRotatef(deg, 0, 0, 1);
    //Screen.setColor(r, g, b, 0.5);
    glColor3f(r, g, b);
    glBegin(GL_TRIANGLE_FAN);
    drawBoxPart(width, height);
    glEnd();
    //Screen.setColor(r, g, b);
    glBegin(GL_LINE_LOOP);
    drawBoxPart(width, height);
    glEnd();
    glPopMatrix();
}

void TextRenderer::drawBoxLine(float x, float y, float width, float height, float deg) {
    glPushMatrix();
    glTranslatef(x - width / 2, y - height / 2, 0);
    glRotatef(deg, 0, 0, 1);
    glBegin(GL_LINE_LOOP);
    drawBoxPart(width, height);
    glEnd();
    glPopMatrix();
}

void TextRenderer::TextRenderer::drawBoxPoly(float x, float y, float width, float height, float deg) {
    glPushMatrix();
    glTranslatef(x - width / 2, y - height / 2, 0);
    glRotatef(deg, 0, 0, 1);
    glBegin(GL_TRIANGLE_FAN);
    drawBoxPart(width, height);
    glEnd();
    glPopMatrix();
}

 void TextRenderer::drawBoxPart(float width, float height) {
    glVertex3f(-width / 2, 0, 0);
    glVertex3f(-width / 3 * 1, -height / 2, 0);
    glVertex3f(width / 3 * 1, -height / 2, 0);
    glVertex3f(width / 2, 0, 0);
    glVertex3f(width / 3 * 1, height / 2, 0);
    glVertex3f(-width / 3 * 1, height / 2, 0);
}

 float TextRenderer::spData[][16][5] =
     {
         {
             {0, 1.15f, 0.65f, 0.3f, 0},
             {-0.6f, 0.55f, 0.65f, 0.3f, 90},
             {0.6f, 0.55f, 0.65f, 0.3f, 90},
             {-0.6f, -0.55f, 0.65f, 0.3f, 90},
             {0.6f, -0.55f, 0.65f, 0.3f, 90},
             {0, -1.15f, 0.65f, 0.3f, 0},
             {0, 0, 0, 0, 99999},
         },
        {
             {0.5f, 0.55f, 0.65f, 0.3f, 90},
             {0.5f, -0.55f, 0.65f, 0.3f, 90},
             {0, 0, 0, 0, 99999},
        },
        {
            {0, 1.15f, 0.65f, 0.3f, 0},
            {0.65f, 0.55f, 0.65f, 0.3f, 90},
            {0, 0, 0.65f, 0.3f, 0},
             {-0.65f, -0.55f, 0.65f, 0.3f, 90},
             {0, -1.15f, 0.65f, 0.3f, 0},
             {0, 0, 0, 0, 99999},
        },
        {
         {0, 1.15f, 0.65f, 0.3f, 0},
             {0.65f, 0.55f, 0.65f, 0.3f, 90},
             {0, 0, 0.65f, 0.3f, 0},
             {0.65f, -0.55f, 0.65f, 0.3f, 90},
             {0, -1.15f, 0.65f, 0.3f, 0},
             {0, 0, 0, 0, 99999},
     }, {
         {-0.65f, 0.55f, 0.65f, 0.3f, 90}, {0.65f, 0.55f, 0.65f, 0.3f, 90},
             {0, 0, 0.65f, 0.3f, 0},
             {0.65f, -0.55f, 0.65f, 0.3f, 90},
             {0, 0, 0, 0, 99999},
     }, {
         {0, 1.15f, 0.65f, 0.3f, 0},
             {-0.65f, 0.55f, 0.65f, 0.3f, 90},
             {0, 0, 0.65f, 0.3f, 0},
             {0.65f, -0.55f, 0.65f, 0.3f, 90},
             {0, -1.15f, 0.65f, 0.3f, 0},
             {0, 0, 0, 0, 99999},
     }, {
         {0, 1.15f, 0.65f, 0.3f, 0},
             {-0.65f, 0.55f, 0.65f, 0.3f, 90},
             {0, 0, 0.65f, 0.3f, 0},
             {-0.65f, -0.55f, 0.65f, 0.3f, 90}, {0.65f, -0.55f, 0.65f, 0.3f, 90},
             {0, -1.15f, 0.65f, 0.3f, 0},
             {0, 0, 0, 0, 99999},
     }, {
         {0, 1.15f, 0.65f, 0.3f, 0},
             {0.65f, 0.55f, 0.65f, 0.3f, 90},
             {0.65f, -0.55f, 0.65f, 0.3f, 90},
             {0, 0, 0, 0, 99999},
     }, {
         {0, 1.15f, 0.65f, 0.3f, 0},
             {-0.65f, 0.55f, 0.65f, 0.3f, 90}, {0.65f, 0.55f, 0.65f, 0.3f, 90},
             {0, 0, 0.65f, 0.3f, 0},
             {-0.65f, -0.55f, 0.65f, 0.3f, 90}, {0.65f, -0.55f, 0.65f, 0.3f, 90},
             {0, -1.15f, 0.65f, 0.3f, 0},
             {0, 0, 0, 0, 99999},
     }, {
         {0, 1.15f, 0.65f, 0.3f, 0},
             {-0.65f, 0.55f, 0.65f, 0.3f, 90}, {0.65f, 0.55f, 0.65f, 0.3f, 90},
             {0, 0, 0.65f, 0.3f, 0},
             {0.65f, -0.55f, 0.65f, 0.3f, 90},
             {0, -1.15f, 0.65f, 0.3f, 0},
             {0, 0, 0, 0, 99999},
     }, {
         //A
         {0, 1.15f, 0.65f, 0.3f, 0},
             {-0.65f, 0.55f, 0.65f, 0.3f, 90}, {0.65f, 0.55f, 0.65f, 0.3f, 90},
             {0, 0, 0.65f, 0.3f, 0},
             {-0.65f, -0.55f, 0.65f, 0.3f, 90}, {0.65f, -0.55f, 0.65f, 0.3f, 90},
             {0, 0, 0, 0, 99999},
     }, {
         {-0.18f, 1.15f, 0.45f, 0.3f, 0},
             {-0.65f, 0.55f, 0.65f, 0.3f, 90}, {0.45f, 0.55f, 0.65f, 0.3f, 90},
             {-0.18f, 0, 0.45f, 0.3f, 0},
             {-0.65f, -0.55f, 0.65f, 0.3f, 90}, {0.65f, -0.55f, 0.65f, 0.3f, 90},
             {0, -1.15f, 0.65f, 0.3f, 0},
             {0, 0, 0, 0, 99999},
     }, {
         {0, 1.15f, 0.65f, 0.3f, 0},
             {-0.65f, 0.55f, 0.65f, 0.3f, 90},
             {-0.65f, -0.55f, 0.65f, 0.3f, 90},
             {0, -1.15f, 0.65f, 0.3f, 0},
             {0, 0, 0, 0, 99999},
     }, {
         {-0.15f, 1.15f, 0.45f, 0.3f, 0},
             {-0.65f, 0.55f, 0.65f, 0.3f, 90}, {0.45f, 0.45f, 0.65f, 0.3f, 90},
             {-0.65f, -0.55f, 0.65f, 0.3f, 90}, {0.65f, -0.55f, 0.65f, 0.3f, 90},
             {0, -1.15f, 0.65f, 0.3f, 0},
             {0, 0, 0, 0, 99999},
     }, {
         {0, 1.15f, 0.65f, 0.3f, 0},
             {-0.65f, 0.55f, 0.65f, 0.3f, 90},
             {0, 0, 0.65f, 0.3f, 0},
             {-0.65f, -0.55f, 0.65f, 0.3f, 90},
             {0, -1.15f, 0.65f, 0.3f, 0},
             {0, 0, 0, 0, 99999},
     }, {// F
         {0, 1.15f, 0.65f, 0.3f, 0},
             {-0.65f, 0.55f, 0.65f, 0.3f, 90},
             {0, 0, 0.65f, 0.3f, 0},
             {-0.65f, -0.55f, 0.65f, 0.3f, 90},
             {0, 0, 0, 0, 99999},
     }, {
         {0, 1.15f, 0.65f, 0.3f, 0},
             {-0.65f, 0.55f, 0.65f, 0.3f, 90},
             {0.05f, 0, 0.3f, 0.3f, 0},
             {-0.65f, -0.55f, 0.65f, 0.3f, 90}, {0.65f, -0.55f, 0.65f, 0.3f, 90},
             {0, -1.15f, 0.65f, 0.3f, 0},
             {0, 0, 0, 0, 99999},
     }, {
         {-0.65f, 0.55f, 0.65f, 0.3f, 90}, {0.65f, 0.55f, 0.65f, 0.3f, 90},
             {0, 0, 0.65f, 0.3f, 0},
             {-0.65f, -0.55f, 0.65f, 0.3f, 90}, {0.65f, -0.55f, 0.65f, 0.3f, 90},
             {0, 0, 0, 0, 99999},
     }, {
         {0, 0.55f, 0.65f, 0.3f, 90},
             {0, -0.55f, 0.65f, 0.3f, 90},
             {0, 0, 0, 0, 99999},
     }, {
         {0.65f, 0.55f, 0.65f, 0.3f, 90},
             {0.65f, -0.55f, 0.65f, 0.3f, 90}, {-0.7f, -0.7f, 0.3f, 0.3f, 90},
             {0, -1.15f, 0.65f, 0.3f, 0},
             {0, 0, 0, 0, 99999},
     }, {//K
         {-0.65f, 0.55f, 0.65f, 0.3f, 90}, {0.4f, 0.55f, 0.65f, 0.3f, 100},
             {-0.25f, 0, 0.45f, 0.3f, 0},
             {-0.65f, -0.55f, 0.65f, 0.3f, 90}, {0.6f, -0.55f, 0.65f, 0.3f, 80},
             {0, 0, 0, 0, 99999},
     }, {
         {-0.65f, 0.55f, 0.65f, 0.3f, 90},
             {-0.65f, -0.55f, 0.65f, 0.3f, 90},
             {0, -1.15f, 0.65f, 0.3f, 0},
             {0, 0, 0, 0, 99999},
     }, {
         {-0.5f, 1.15f, 0.3f, 0.3f, 0}, {0.1f, 1.15f, 0.3f, 0.3f, 0},
             {-0.65f, 0.55f, 0.65f, 0.3f, 90}, {0.65f, 0.55f, 0.65f, 0.3f, 90},
             {-0.65f, -0.55f, 0.65f, 0.3f, 90}, {0.65f, -0.55f, 0.65f, 0.3f, 90},
             {0, 0.55f, 0.65f, 0.3f, 90},
             {0, -0.55f, 0.65f, 0.3f, 90},
             {0, 0, 0, 0, 99999},
     }, {
         {0, 1.15f, 0.65f, 0.3f, 0},
             {-0.65f, 0.55f, 0.65f, 0.3f, 90}, {0.65f, 0.55f, 0.65f, 0.3f, 90},
             {-0.65f, -0.55f, 0.65f, 0.3f, 90}, {0.65f, -0.55f, 0.65f, 0.3f, 90},
             {0, 0, 0, 0, 99999},
     }, {
         {0, 1.15f, 0.65f, 0.3f, 0},
             {-0.65f, 0.55f, 0.65f, 0.3f, 90}, {0.65f, 0.55f, 0.65f, 0.3f, 90},
             {-0.65f, -0.55f, 0.65f, 0.3f, 90}, {0.65f, -0.55f, 0.65f, 0.3f, 90},
             {0, -1.15f, 0.65f, 0.3f, 0},
             {0, 0, 0, 0, 99999},
     }, {//P
         {0, 1.15f, 0.65f, 0.3f, 0},
             {-0.65f, 0.55f, 0.65f, 0.3f, 90}, {0.65f, 0.55f, 0.65f, 0.3f, 90},
             {0, 0, 0.65f, 0.3f, 0},
             {-0.65f, -0.55f, 0.65f, 0.3f, 90},
             {0, 0, 0, 0, 99999},
     }, {
         {0, 1.15f, 0.65f, 0.3f, 0},
             {-0.65f, 0.55f, 0.65f, 0.3f, 90}, {0.65f, 0.55f, 0.65f, 0.3f, 90},
             {-0.65f, -0.55f, 0.65f, 0.3f, 90}, {0.65f, -0.55f, 0.65f, 0.3f, 90},
             {0, -1.15f, 0.65f, 0.3f, 0},
             {0.05f, -0.55f, 0.45f, 0.3f, 60},
             {0, 0, 0, 0, 99999},
     }, {
         {0, 1.15f, 0.65f, 0.3f, 0},
             {-0.65f, 0.55f, 0.65f, 0.3f, 90}, {0.65f, 0.55f, 0.65f, 0.3f, 90},
             {-0.2f, 0, 0.45f, 0.3f, 0},
             {-0.65f, -0.55f, 0.65f, 0.3f, 90}, {0.45f, -0.55f, 0.65f, 0.3f, 80},
             {0, 0, 0, 0, 99999},
     }, {
         {0, 1.15f, 0.65f, 0.3f, 0},
             {-0.65f, 0.55f, 0.65f, 0.3f, 90},
             {0, 0, 0.65f, 0.3f, 0},
             {0.65f, -0.55f, 0.65f, 0.3f, 90},
             {0, -1.15f, 0.65f, 0.3f, 0},
             {0, 0, 0, 0, 99999},
     }, {
         {-0.5f, 1.15f, 0.55f, 0.3f, 0}, {0.5f, 1.15f, 0.55f, 0.3f, 0},
             {0.1f, 0.55f, 0.65f, 0.3f, 90},
             {0.1f, -0.55f, 0.65f, 0.3f, 90},
             {0, 0, 0, 0, 99999},
     }, {//U
         {-0.65f, 0.55f, 0.65f, 0.3f, 90}, {0.65f, 0.55f, 0.65f, 0.3f, 90},
             {-0.65f, -0.55f, 0.65f, 0.3f, 90}, {0.65f, -0.55f, 0.65f, 0.3f, 90},
             {0, -1.15f, 0.65f, 0.3f, 0},
             {0, 0, 0, 0, 99999},
     }, {
         {-0.65f, 0.55f, 0.65f, 0.3f, 90}, {0.65f, 0.55f, 0.65f, 0.3f, 90},
             {-0.5f, -0.55f, 0.65f, 0.3f, 90}, {0.5f, -0.55f, 0.65f, 0.3f, 90},
             {-0.1f, -1.15f, 0.45f, 0.3f, 0},
             {0, 0, 0, 0, 99999},
     }, {
         {-0.65f, 0.55f, 0.65f, 0.3f, 90}, {0.65f, 0.55f, 0.65f, 0.3f, 90},
             {-0.65f, -0.55f, 0.65f, 0.3f, 90}, {0.65f, -0.55f, 0.65f, 0.3f, 90},
             {-0.5f, -1.15f, 0.3f, 0.3f, 0}, {0.1f, -1.15f, 0.3f, 0.3f, 0},
             {0, 0.55f, 0.65f, 0.3f, 90},
             {0, -0.55f, 0.65f, 0.3f, 90},
             {0, 0, 0, 0, 99999},
     }, {
         {-0.4f, 0.6f, 0.85f, 0.3f, 360 - 120},
             {0.4f, 0.6f, 0.85f, 0.3f, 360 - 60},
             {-0.4f, -0.6f, 0.85f, 0.3f, 360 - 240},
             {0.4f, -0.6f, 0.85f, 0.3f, 360 - 300},
             {0, 0, 0, 0, 99999},
     }, {
         {-0.4f, 0.6f, 0.85f, 0.3f, 360 - 120},
             {0.4f, 0.6f, 0.85f, 0.3f, 360 - 60},
             {-0.1f, -0.55f, 0.65f, 0.3f, 90},
             {0, 0, 0, 0, 99999},
     }, {
         {0, 1.15f, 0.65f, 0.3f, 0},
             {0.3f, 0.4f, 0.65f, 0.3f, 120},
             {-0.3f, -0.4f, 0.65f, 0.3f, 120},
             {0, -1.15f, 0.65f, 0.3f, 0},
             {0, 0, 0, 0, 99999},
     }, {//.
         {0, -1.15f, 0.3f, 0.3f, 0},
             {0, 0, 0, 0, 99999},
     }, {//_
         {0, -1.15f, 0.8f, 0.3f, 0},
             {0, 0, 0, 0, 99999},
     }, {//-
         {0, 0, 0.9f, 0.3f, 0},
             {0, 0, 0, 0, 99999},
     }, {//+
         {-0.5f, 0, 0.45f, 0.3f, 0}, {0.45f, 0, 0.45f, 0.3f, 0},
             {0.1f, 0.55f, 0.65f, 0.3f, 90},
             {0.1f, -0.55f, 0.65f, 0.3f, 90},
             {0, 0, 0, 0, 99999},
     }, {//'
         {0, 1.0f, 0.4f, 0.2f, 90},
             {0, 0, 0, 0, 99999},
     }, {//''
         {-0.19f, 1.0f, 0.4f, 0.2f, 90},
             {0.2f, 1.0f, 0.4f, 0.2f, 90},
             {0, 0, 0, 0, 99999},
     }, {//!
         {0.56f, 0.25f, 1.1f, 0.3f, 90},
             {0, -1.0f, 0.3f, 0.3f, 90},
             {0, 0, 0, 0, 99999},
     }, {// /
         {0.8f, 0, 1.75f, 0.3f, 120},
             {0, 0, 0, 0, 99999},
     } };