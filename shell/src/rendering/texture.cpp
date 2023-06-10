#include "texture.hpp"

#include "rendering/error.hpp"
#include "misc/exception.hpp"

#include <stdexcept>

namespace sh {

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
    // clamp to edge required for NPOT textures on emscripten
    // might as well set it on other platforms as well
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#ifndef NDEBUG
    if (checkGLError())
    {
        return false;
    }
#endif
    m_width = i_width;
    m_height = i_height;

    return true;
}

bool
Texture::from_black_frame(int i_width, int i_height)
{
    if (!genTexIf(i_width, i_height))
    {
        return false;
    }

    int err = 0;
    // Don't use stack storage, e.g. on Emscripten, max stack size is 64*1024
    NHByte *data = nullptr;
    SH_TRY
    {
        data = new NHByte[i_width * i_height * 3]{};
    }
    SH_CATCH(const std::exception &)
    {
        err = 1;
        goto l_end;
    }

    /* update input texture */
    glBindTexture(GL_TEXTURE_2D, m_tex);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, i_width, i_height, GL_RGB,
                    GL_UNSIGNED_BYTE, data);
#ifndef NDEBUG
    if (checkGLError())
    {
        err = 1;
        goto l_end;
    }
#endif

l_end:
    if (data)
    {
        delete[] data;
    }
    return !err;
}

bool
Texture::from_frame(NHFrame i_frame)
{
    auto width = nh_frm_width(i_frame);
    auto height = nh_frm_height(i_frame);
    if (!genTexIf(width, height))
    {
        return false;
    }

    /* update input texture */
    glBindTexture(GL_TEXTURE_2D, m_tex);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB,
                    GL_UNSIGNED_BYTE, nh_frm_data(i_frame));
#ifndef NDEBUG
    if (checkGLError())
    {
        return false;
    }
#endif

    return true;
}

bool
Texture::from_ptn_tbl(NHDPatternTable i_tbl)
{
    auto w = nhd_ptn_table_width(i_tbl);
    auto h = nhd_ptn_table_height(i_tbl);
    if (!genTexIf(w, h))
    {
        return false;
    }

    /* update input texture */
    glBindTexture(GL_TEXTURE_2D, m_tex);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE,
                    nhd_ptn_table_data(i_tbl));
#ifndef NDEBUG
    if (checkGLError())
    {
        return false;
    }
#endif

    return true;
}

bool
Texture::from_sprite(NHDSprite i_sprite)
{
    auto w = nhd_sprite_width(i_sprite);
    auto h = nhd_sprite_height(i_sprite);
    if (!genTexIf(w, h))
    {
        return false;
    }

    /* update input texture */
    glBindTexture(GL_TEXTURE_2D, m_tex);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE,
                    nhd_sprite_data(i_sprite));
#ifndef NDEBUG
    if (checkGLError())
    {
        return false;
    }
#endif

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

} // namespace sh
