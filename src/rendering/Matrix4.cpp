#include "Matrix4.h"

#include <cmath>
#include <stdexcept>

namespace
{
constexpr float Pi = 3.14159265358979323846f;
}

Matrix4::Matrix4() : values{{1, 0, 0, 0, 0, 1, 0, 0,
                              0, 0, 1, 0, 0, 0, 0, 1}} {}

Matrix4::Matrix4(const std::array<float, 16>& source) : values(source) {}

Matrix4 Matrix4::Identity() { return Matrix4(); }

Matrix4 Matrix4::Translation(float x, float y, float z)
{
    Matrix4 result;
    result.values[12] = x; result.values[13] = y; result.values[14] = z;
    return result;
}

Matrix4 Matrix4::Scale(float x, float y, float z)
{
    Matrix4 result;
    result.values[0] = x; result.values[5] = y; result.values[10] = z;
    return result;
}

Matrix4 Matrix4::Rotation(float degrees, float x, float y, float z)
{
    const float length = std::sqrt(x * x + y * y + z * z);
    if (length == 0.0f)
        return Identity();
    x /= length; y /= length; z /= length;
    const float radians = degrees * Pi / 180.0f;
    const float c = std::cos(radians), s = std::sin(radians), t = 1.0f - c;
    return Matrix4({{t*x*x+c, t*x*y+s*z, t*x*z-s*y, 0,
                     t*x*y-s*z, t*y*y+c, t*y*z+s*x, 0,
                     t*x*z+s*y, t*y*z-s*x, t*z*z+c, 0,
                     0, 0, 0, 1}});
}

Matrix4 Matrix4::Perspective(float fov, float aspect, float nearPlane, float farPlane)
{
    if (aspect == 0.0f || nearPlane <= 0.0f || farPlane <= nearPlane)
        throw std::invalid_argument("Invalid perspective parameters");
    const float f = 1.0f / std::tan(fov * Pi / 360.0f);
    return Matrix4({{f/aspect, 0, 0, 0, 0, f, 0, 0,
                     0, 0, (farPlane+nearPlane)/(nearPlane-farPlane), -1,
                     0, 0, (2*farPlane*nearPlane)/(nearPlane-farPlane), 0}});
}

Matrix4 Matrix4::LookAt(float ex, float ey, float ez, float cx, float cy, float cz,
                        float ux, float uy, float uz)
{
    float fx=cx-ex, fy=cy-ey, fz=cz-ez;
    float length=std::sqrt(fx*fx+fy*fy+fz*fz);
    if (length == 0.0f) throw std::invalid_argument("LookAt eye and center must differ");
    fx/=length; fy/=length; fz/=length;
    float sx=fy*uz-fz*uy, sy=fz*ux-fx*uz, sz=fx*uy-fy*ux;
    length=std::sqrt(sx*sx+sy*sy+sz*sz);
    if (length == 0.0f) throw std::invalid_argument("LookAt up vector is parallel to view");
    sx/=length; sy/=length; sz/=length;
    const float tx=sy*fz-sz*fy, ty=sz*fx-sx*fz, tz=sx*fy-sy*fx;
    Matrix4 orientation({{sx,tx,-fx,0, sy,ty,-fy,0, sz,tz,-fz,0, 0,0,0,1}});
    return orientation * Translation(-ex, -ey, -ez);
}

Matrix4 Matrix4::operator*(const Matrix4& rhs) const
{
    std::array<float,16> result{{}};
    for (int column=0; column<4; ++column)
        for (int row=0; row<4; ++row)
            for (int k=0; k<4; ++k)
                result[column*4+row] += (*this)(row,k) * rhs(k,column);
    return Matrix4(result);
}
