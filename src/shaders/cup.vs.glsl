#version 330 core

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_vertexCoords;
out vec2 vertexCoords;

uniform mat4 mvp;
uniform int cupIndex;
uniform int isCup;

void main() {
    gl_Position = mvp * vec4(in_position, 1.0);

    const float cupWidth = 1.0/3.0;
    const float cupHeight = 1.0/2.0;
    const float plateWidth = 1.0/6.0;
    const float plateHeight = 1.0/4.0;
    const float plateWidthStart = 2.0/3.0;
    const float plateHeightStart = 1.0/2.0;
    
    float heightLeap = isCup == 1 ? cupHeight : plateHeight;
    float widthLeap = isCup == 1 ? cupWidth : plateWidth;
    
    int row = (cupIndex < 2) ? 0 : 1;
    int column = (cupIndex % 2 == 0) ? 0 : 1;

    float x = in_vertexCoords[0] + (column * widthLeap);
    float y = in_vertexCoords[1] + (row * heightLeap);

    if (isCup == 0){
        x += plateWidthStart;
        y += plateHeightStart;
    }
    
    
}