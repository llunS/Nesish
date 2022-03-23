#include "renderer.hpp"

#include "glfw_app/resources/shader/triangle_vert.hpp"
#include "glfw_app/resources/shader/triangle_frag.hpp"
#include "glfw_app/rendering/error.hpp"

namespace ln_app {

Renderer::Renderer()
    : m_vbo(0)
    , m_vao(0)
    , m_fbo(0)
    , m_texture(0)
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
    if (m_texture)
    {
        glDeleteTextures(1, &m_texture);
    }
    m_texture = 0;
}

ln::Error
Renderer::setup()
{
    ln::Error err = ln::Error::OK;

    err = m_shader.compile(g_triangle_vert, g_triangle_frag);
    if (LN_FAILED(err))
    {
        goto l_cleanup;
    }

    static constexpr struct {
        float x, y;
    } vertices[3] = {{-0.6f, -0.4f}, {0.6f, -0.4f}, {0.0f, 0.6f}};
    glGenBuffers(1, &m_vbo);
    if (checkGLError())
    {
        err = ln::Error::RENDERING_API;
        goto l_cleanup;
    }
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices,
                     GL_STATIC_DRAW);
    }

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
                              sizeof(vertices[0]), (void *)0);

        glBindVertexArray(0);
    }

    {
        glUseProgram(m_shader.program());

        GLint clr_location = glGetUniformLocation(m_shader.program(), "uColor");
        static constexpr float color[] = {
            (float)0xFF / 0xFF, (float)0x63 / 0xFF, (float)0x47 / 0xFF, 1.0f};
        glUniform4fv(clr_location, 1, color);
    }

    {
        glGenTextures(1, &m_texture);
        if (checkGLError())
        {
            err = ln::Error::RENDERING_API;
            goto l_cleanup;
        }
        glBindTexture(GL_TEXTURE_2D, m_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, get_width(), get_height(), 0,
                     GL_RGB, GL_UNSIGNED_BYTE, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    {
        glGenFramebuffers(1, &m_fbo);
        if (checkGLError())
        {
            err = ln::Error::RENDERING_API;
            goto l_cleanup;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, m_texture, 0);
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
    return 800;
}

int
Renderer::get_height()
{
    return 600;
}

void
Renderer::draw()
{
    glViewport(0, 0, get_width(), get_height());

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    {
        glClearColor(0.0f, (float)0x8B / 0xFF, (float)0x8B / 0xFF, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(m_shader.program());
        glBindVertexArray(m_vao);

        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLuint
Renderer::texture() const
{
    return m_texture;
}

} // namespace ln_app
