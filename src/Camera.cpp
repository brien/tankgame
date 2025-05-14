#include "Camera.h"

Camera::Camera()
{
    _xpos = 0;
    _ypos = 0;
    _zpos = 0;
    _xfocus = 0;
    _yfocus = 0;
    _zfocus = 0;

    ydist = 1.0;
    xzdist = 2;
}

Camera::~Camera()
{
}

void Camera::SetPos(float x, float y, float z)
{
    _xpos = x;
    _ypos = y;
    _zpos = z;
}

void Camera::SetFocus(float x, float y, float z)
{
    _xfocus = x;
    _yfocus = y;
    _zfocus = z;
}
