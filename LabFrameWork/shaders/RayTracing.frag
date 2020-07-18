#version 430 core
in vec2 texCoord;

uniform sampler2D tex;
out vec4 FragColors;

void main()
{
    FragColors = texture(tex, texCoord);
}