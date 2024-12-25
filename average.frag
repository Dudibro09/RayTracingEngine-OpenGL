#version 460 core

out vec4 FragColor;
in vec2 texCoords;

uniform sampler2D renderTex;
uniform sampler2D finalRenderTex;
uniform uint frame;

void main()
{
    vec3 renderColor = texture(renderTex, texCoords.st).rgb;
    vec3 finalRenderColor = texture(finalRenderTex, texCoords.st).rgb;

    vec3 averageColor = finalRenderColor * (1.0f - 1.0f / (frame + 1.0f)) + renderColor / (frame + 1.0f);

    FragColor = vec4(averageColor, 1.0f);
}