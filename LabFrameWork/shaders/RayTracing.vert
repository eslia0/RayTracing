#version 430 core
layout (location = 0) in vec2 aPosition;
layout (location = 1) in vec2 aTexCoord;

out vec2 texCoord;

void main()
{
    vec4 pos = vec4(aPosition, 1.0F, 1.0F);
    gl_Position = pos;

    texCoord = aTexCoord;
}