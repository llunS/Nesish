#version 330

uniform sampler2D uTex;

in vec2 aCoordUV;

out vec4 oFragColor;

void
main()
{
    oFragColor = vec4(texture(uTex, aCoordUV).rgb, 1.0f);
}
