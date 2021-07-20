#ifndef Camera_h
#define Camera_h

class Camera
{
public:
    Camera();
    ~Camera();

    float xpos;
    float ypos;
    float zpos;

    float xfocus;
    float yfocus;
    float zfocus;

    float xzdist;
    float ydist;
};

#endif /* Camera_h */