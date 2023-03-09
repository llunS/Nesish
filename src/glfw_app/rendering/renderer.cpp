#include "renderer.hpp"

#include <cstddef>

#include "glfw_app/resources/shader/screen_rect_vert.hpp"
#include "glfw_app/resources/shader/screen_rect_frag.hpp"
#include "glfw_app/rendering/error.hpp"

#include "console/ppu/frame_buffer.hpp"

namespace sh {

static constexpr int VERT_COUNT = 6;

Renderer::Renderer()
    : m_vbo(0)
    , m_vao(0)
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

    m_tex.cleanup();
}

nh::Error
Renderer::setup()
{
    nh::Error err = nh::Error::OK;

    err = m_shader.compile(g_screen_rect_vert, g_screen_rect_frag);
    if (LN_FAILED(err))
    {
        goto l_end;
    }

    /* VBO */
    struct Vertex {
        float x, y;
        float u, v;
    };
    glGenBuffers(1, &m_vbo);
    if (checkGLError())
    {
        err = nh::Error::RENDERING_API;
        goto l_end;
    }
    {
        // Data in nh::FrameBuffer originate from upper left, while OpenGL
        // texture coordinate originates from lower left, we flip the Y texture
        // coordinate here for simplicity.
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
        err = nh::Error::RENDERING_API;
        goto l_end;
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

l_end:
    if (LN_FAILED(err))
    {
        cleanup();
    }
    return err;
}

void
Renderer::render(const nh::FrameBuffer &i_frame_buf)
{
    /* update input texture with "i_frame_buf" */
    if (!m_tex.from_frame(i_frame_buf))
    {
        return;
    }
    /* draw */
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m_tex.texture());

            glUseProgram(m_shader.program());

            glBindVertexArray(m_vao);
            glDrawArrays(GL_TRIANGLES, 0, VERT_COUNT);
        }
    }
}

} // namespace sh
