#ifndef Camera_h
#define Camera_h

class Camera
{
private:
    float _xpos;
    float _ypos;
    float _zpos;
public:
    Camera();
    ~Camera();

    const float xpos() const { return _xpos; }
    const float ypos() const { return _ypos; }
    const float zpos() const { return _zpos; }

    void SetPos(float x, float y, float z);

    float xfocus;
    float yfocus;
    float zfocus;

    float xzdist;
    float ydist;
};

#endif /* Camera_h */