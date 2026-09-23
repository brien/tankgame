#pragma once

#include <array>

// Backend-neutral, OpenGL-compatible 4x4 matrix. Values are column-major and
// matrices post-multiply, so A * B applies B to a column vector before A.
class Matrix4
{
public:
    Matrix4();

    static Matrix4 Identity();
    static Matrix4 Translation(float x, float y, float z);
    static Matrix4 Rotation(float degrees, float x, float y, float z);
    static Matrix4 Scale(float x, float y, float z);
    static Matrix4 Perspective(float verticalFovDegrees, float aspect,
                               float nearPlane, float farPlane);
    static Matrix4 LookAt(float eyeX, float eyeY, float eyeZ,
                          float centerX, float centerY, float centerZ,
                          float upX, float upY, float upZ);

    Matrix4 operator*(const Matrix4& rhs) const;
    const float* Data() const { return values.data(); }
    float operator()(int row, int column) const { return values[column * 4 + row]; }

private:
    explicit Matrix4(const std::array<float, 16>& values);
    std::array<float, 16> values;
};
