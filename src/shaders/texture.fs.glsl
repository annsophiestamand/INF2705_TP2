#version 330 core

in vec2 vertexCoords;
out vec4 FragColor;

uniform sampler2D texture;

void main()
{
    vec4 texColor = texture(texture, vertexCoords);
    FragColor = texColor;
}
