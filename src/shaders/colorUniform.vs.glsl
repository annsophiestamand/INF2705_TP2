#version 330 core

// TODO
uniform mat4 mat;
layout (location = 0) in vec3 aPos;

void main()
{
    gl_Position = mat * vec4(aPos, 1.0);
}