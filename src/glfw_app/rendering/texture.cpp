#include "texture.hpp"

#include "console/ppu/frame_buffer.hpp"

namespace ln_app {

Texture::Texture()
    : m_tex(0)
    , m_width(-1)
    , m_height(-1)
{
}

Texture::~Texture()
{
    cleanup();
}

void
Texture::cleanup()
{
    if (m_tex)
    {
        glDeleteTextures(1, &m_tex);
    }
    m_tex = 0;
}

bool
Texture::genTexIf(int i_width, int i_height)
{
    if (i_width == m_width && i_height == m_height)
    {
        return true;
    }

    cleanup();

    /* input texture for screen rect */
    glGenTextures(1, &m_tex);
    glBindTexture(GL_TEXTURE_2D, m_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, i_width, i_height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    m_width = i_width;
    m_height = i_height;

    return true;
}

bool
Texture::from_frame(const ln::FrameBuffer &i_frame_buf)
{
    if (!genTexIf(i_frame_buf.WIDTH, i_frame_buf.HEIGHT))
    {
        return false;
    }

    /* update input texture with "i_frame_buf" */
    glBindTexture(GL_TEXTURE_2D, m_tex);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, i_frame_buf.WIDTH,
                    i_frame_buf.HEIGHT, GL_RGB, GL_UNSIGNED_BYTE,
                    i_frame_buf.get_data());

    return true;
}

int
Texture::get_width()
{
    return m_width;
}

int
Texture::get_height()
{
    return m_height;
}

GLuint
Texture::texture() const
{
    return m_tex;
}

} // namespace ln_app
