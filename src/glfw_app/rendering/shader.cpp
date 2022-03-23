#include "shader.hpp"

#include "glfw_app/rendering/error.hpp"

#include <cstdio>

namespace ln_app {

Shader::Shader()
    : m_id(0)
{
}

Shader::~Shader()
{
    if (m_id)
    {
        glDeleteProgram(m_id);
    }
    m_id = 0;
}

GLuint
Shader::program() const
{
    return m_id;
}

ln::Error
Shader::compile(const char *i_vert, const char *i_frag)
{
    ln::Error err = ln::Error::OK;

    GLuint vertex_shader = 0;
    GLuint fragment_shader = 0;
    GLuint program = 0;

    /* vert */
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    if (checkGLError())
    {
        err = ln::Error::RENDERING_API;
        goto l_cleanup;
    }
    glShaderSource(vertex_shader, 1, &i_vert, NULL);
    glCompileShader(vertex_shader);
    {
        int success = 0;
        glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            char infoLog[512];
            glGetShaderInfoLog(vertex_shader, 512, NULL, infoLog);
            std::fprintf(stderr,
                         "ERROR::SHADER::VERTEX::COMPILATION_FAILED: %s\n",
                         infoLog);

            err = ln::Error::RENDERING_API;
            goto l_cleanup;
        };
    }

    /* frag */
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    if (checkGLError())
    {
        err = ln::Error::RENDERING_API;
        goto l_cleanup;
    }
    glShaderSource(fragment_shader, 1, &i_frag, NULL);
    glCompileShader(fragment_shader);
    {
        int success = 0;
        glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            char infoLog[512];
            glGetShaderInfoLog(fragment_shader, 512, NULL, infoLog);
            std::fprintf(stderr,
                         "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED: %s\n",
                         infoLog);

            err = ln::Error::RENDERING_API;
            goto l_cleanup;
        };
    }

    /* program */
    program = glCreateProgram();
    if (checkGLError())
    {
        err = ln::Error::RENDERING_API;
        goto l_cleanup;
    }
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    {
        int success = 0;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success)
        {
            char infoLog[512];
            glGetProgramInfoLog(program, 512, NULL, infoLog);
            std::fprintf(stderr, "ERROR::SHADER::PROGRAM::LINKING_FAILED: %s\n",
                         infoLog);

            err = ln::Error::RENDERING_API;
            goto l_cleanup;
        }
    }

l_cleanup:
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    if (LN_FAILED(err))
    {
        if (program)
        {
            glDeleteProgram(program);
        }
        program = 0;
    }

    if (program)
    {
        m_id = program;
    }

    return err;
}

} // namespace ln_app
