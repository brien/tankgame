#include "Camera.h"

Camera::Camera()
{
    _xpos = 0;
    _ypos = 0;
    _zpos = 0;
    xfocus = 0;
    yfocus = 0;
    zfocus = 0;

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
