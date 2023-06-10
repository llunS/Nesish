#version 330

in vec2 vPos; // In NDC space
in vec2 vCoordUV;

out vec2 aCoordUV;

void
main()
{
    gl_Position = vec4(vPos, 0.0f, 1.0f);
    aCoordUV = vCoordUV;
}
