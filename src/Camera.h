#ifndef Camera_h
#define Camera_h

class Camera
{
private:
    float _xpos;
    float _ypos;
    float _zpos;

    float _xfocus;
    float _yfocus;
    float _zfocus;

public:
    Camera();
    ~Camera();

    const float xpos() const { return _xpos; }
    const float ypos() const { return _ypos; }
    const float zpos() const { return _zpos; }

    const float xfocus() const { return _xfocus; }
    const float yfocus() const { return _yfocus; }
    const float zfocus() const { return _zfocus; }

    void SetPos(float x, float y, float z);
    void SetFocus(float x, float y, float z);

    float xzdist;
    float ydist;
};

#endif /* Camera_h */