#include "GpuTexture.h"
#include "PlatformGL.h"
#include "GLFunctions.h"

#include <stdexcept>

class GpuTexture::Implementation { public: GLuint handle = 0; };

GpuTexture::GpuTexture(const ImageData& image, TextureUploadOptions options)
    : implementation(new Implementation)
{
    if (!image.IsValid()) throw std::invalid_argument("cannot upload invalid image data");
    const PortableTextureSettings settings = ChoosePortableTextureSettings(image, options);
    const GLenum format = image.format == PixelFormat::RGBA8 ? GL_RGBA : GL_RGB;
    glGenTextures(1, &implementation->handle);
    glBindTexture(GL_TEXTURE_2D, implementation->handle);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, format, image.width, image.height, 0,
                 format, GL_UNSIGNED_BYTE, image.pixels.data());
    if (settings.mipmaps) GLFunctions::GenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    settings.mipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    const GLint wrap = settings.repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
}

GpuTexture::~GpuTexture()
{
    if (implementation->handle) glDeleteTextures(1, &implementation->handle);
}

void GpuTexture::Bind(unsigned int unit) const
{
    GLFunctions::ActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, implementation->handle);
}

unsigned int GpuTexture::CompatibilityHandle() const { return implementation->handle; }
