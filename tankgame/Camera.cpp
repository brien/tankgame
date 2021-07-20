#include "Camera.h"

Camera::Camera()
{
    xpos = 0;
    ypos = 0;
    zpos = 0;
    xfocus = 0;
    yfocus = 0;
    zfocus = 0;

    ydist = 1.0;
    xzdist = 2;
}

Camera::~Camera()
{

}