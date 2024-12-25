#version 460 core

out vec4 FragColor;
in vec2 texCoords;

uniform sampler2D tex;

void main()
{
    vec4 color = texture(tex, texCoords.st);

    color.x = clamp(color.x, 0.0f, 1.0f);
    color.y = clamp(color.y, 0.0f, 1.0f);
    color.z = clamp(color.z, 0.0f, 1.0f);

    FragColor = vec4(color.rgb, 1.0f);
}