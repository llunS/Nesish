#include "renderer.hpp"

#include <cstddef>

#include "glfw_app/resources/shader/screen_rect_vert.hpp"
#include "glfw_app/resources/shader/screen_rect_frag.hpp"
#include "glfw_app/rendering/error.hpp"

#include "console/ppu/frame_buffer.hpp"

namespace ln_app {

static constexpr int SCALE = 2;
static constexpr int VERT_COUNT = 6;

Renderer::Renderer()
    : m_vbo(0)
    , m_vao(0)
    , m_tex(0)
    , m_fbo(0)
    , m_fb_tex(0)
{
}
Renderer::~Renderer()
{
    cleanup();
}

void
Renderer::cleanup()
{
    if (m_vbo)
    {
        glDeleteBuffers(1, &m_vbo);
    }
    m_vbo = 0;
    if (m_vao)
    {
        glDeleteVertexArrays(1, &m_vao);
    }
    m_vao = 0;
    if (m_fbo)
    {
        glDeleteFramebuffers(1, &m_fbo);
    }
    m_fbo = 0;
    if (m_fb_tex)
    {
        glDeleteTextures(1, &m_fb_tex);
    }
    m_fb_tex = 0;
    if (m_tex)
    {
        glDeleteTextures(1, &m_tex);
    }
    m_tex = 0;
}

ln::Error
Renderer::setup()
{
    ln::Error err = ln::Error::OK;

    err = m_shader.compile(g_screen_rect_vert, g_screen_rect_frag);
    if (LN_FAILED(err))
    {
        goto l_cleanup;
    }

    /* VBO */
    struct Vertex {
        float x, y;
        float u, v;
    };
    glGenBuffers(1, &m_vbo);
    if (checkGLError())
    {
        err = ln::Error::RENDERING_API;
        goto l_cleanup;
    }
    {
        // @IMPL: Data in ln::FrameBuffer originate from upper left, while
        // OpenGL texture coordinate originates from lower left, we flip the Y
        // texture coordinate here for simplicity.
        constexpr Vertex BL = {-1.0f, -1.0f, 0.0f, 1.0f};
        constexpr Vertex BR = {1.0f, -1.0f, 1.0f, 1.0f};
        constexpr Vertex TR = {1.0f, 1.0f, 1.0f, 0.0f};
        constexpr Vertex TL = {-1.0f, 1.0f, 0.0f, 0.0f};
        constexpr Vertex vertices[VERT_COUNT] = {BL, BR, TR, TR, TL, BL};

        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices,
                     GL_STATIC_DRAW);
    }
    /* VAO */
    glGenVertexArrays(1, &m_vao);
    if (checkGLError())
    {
        err = ln::Error::RENDERING_API;
        goto l_cleanup;
    }
    {
        glBindVertexArray(m_vao);

        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

        GLuint vpos_location = glGetAttribLocation(m_shader.program(), "vPos");
        glEnableVertexAttribArray(vpos_location);
        glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE,
                              sizeof(Vertex), (void *)0);
        GLuint vuv_location =
            glGetAttribLocation(m_shader.program(), "vCoordUV");
        glEnableVertexAttribArray(vuv_location);
        glVertexAttribPointer(vuv_location, 2, GL_FLOAT, GL_FALSE,
                              sizeof(Vertex), (void *)(offsetof(Vertex, u)));

        glBindVertexArray(0);
    }
    /* Shader setup */
    {
        glUseProgram(m_shader.program());
        // bind texture to unit 0
        auto tex_loc = glGetUniformLocation(m_shader.program(), "uTex");
        glUniform1i(tex_loc, 0);
    }
    /* input texture for screen rect */
    {
        glGenTextures(1, &m_tex);
        if (checkGLError())
        {
            err = ln::Error::RENDERING_API;
            goto l_cleanup;
        }
        glBindTexture(GL_TEXTURE_2D, m_tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ln::FrameBuffer::WIDTH,
                     ln::FrameBuffer::HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
        // Filter mode doesn't really matter since we keep both the input and
        // the output at the same size.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    /* texture for framebuffer */
    {
        glGenTextures(1, &m_fb_tex);
        if (checkGLError())
        {
            err = ln::Error::RENDERING_API;
            goto l_cleanup;
        }
        glBindTexture(GL_TEXTURE_2D, m_fb_tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ln::FrameBuffer::WIDTH,
                     ln::FrameBuffer::HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
        // Box filtering to look sharper, as the NES hardware would look like.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
    /* framebuffer */
    {
        glGenFramebuffers(1, &m_fbo);
        if (checkGLError())
        {
            err = ln::Error::RENDERING_API;
            goto l_cleanup;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, m_fb_tex, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

l_cleanup:
    if (LN_FAILED(err))
    {
        cleanup();
    }
    return err;
}

int
Renderer::get_width()
{
    return ln::FrameBuffer::WIDTH * SCALE;
}

int
Renderer::get_height()
{
    return ln::FrameBuffer::HEIGHT * SCALE;
}

void
Renderer::render(const ln::FrameBuffer &i_frame_buf)
{
    /* update input texture with "i_frame_buf" */
    {
        glBindTexture(GL_TEXTURE_2D, m_tex);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, ln::FrameBuffer::WIDTH,
                        ln::FrameBuffer::HEIGHT, GL_RGB, GL_UNSIGNED_BYTE,
                        i_frame_buf.get_data());
    }
    /* draw */
    {
        // This viewport call is needed to fit the texture to the framebuffer.
        glViewport(0, 0, ln::FrameBuffer::WIDTH, ln::FrameBuffer::HEIGHT);

        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        {
            glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m_tex);

            glUseProgram(m_shader.program());

            glBindVertexArray(m_vao);
            glDrawArrays(GL_TRIANGLES, 0, VERT_COUNT);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

void
Renderer::render_direct(const ln::FrameBuffer &i_frame_buf)
{
    /* update input texture with "i_frame_buf" */
    {
        glBindTexture(GL_TEXTURE_2D, m_tex);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, ln::FrameBuffer::WIDTH,
                        ln::FrameBuffer::HEIGHT, GL_RGB, GL_UNSIGNED_BYTE,
                        i_frame_buf.get_data());
    }
    /* draw */
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m_tex);

            glUseProgram(m_shader.program());

            glBindVertexArray(m_vao);
            glDrawArrays(GL_TRIANGLES, 0, VERT_COUNT);
        }
    }
}

GLuint
Renderer::texture() const
{
    return m_fb_tex;
}

} // namespace ln_app
