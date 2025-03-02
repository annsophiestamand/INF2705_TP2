#version 330 core

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_vertexCoords;
out vec2 vertexCoords;

uniform mat4 mvp;
uniform int cupIndex;
uniform int isPlate;

void main() {
    gl_Position = mvp * vec4(in_position, 1.0);

    const float CUP_WIDTH = 1.0/3.0;
    const float CUP_HEIGHT = 1.0/2.0;
    const float PLATE_WIDTH = 1.0/6.0;
    const float PLATE_HEIGHT = 1.0/4.0;
    const float PLATE_WIDTH_START = 2.0/3.0;
    const float PLATE_HEIGHT_START = 1.0/2.0;
    
    float heightLeap = isPlate == 0 ? CUP_HEIGHT : PLATE_HEIGHT;
    float widthLeap = isPlate == 0 ? CUP_WIDTH : PLATE_WIDTH;
    
    int row = (cupIndex < 2) ? 0 : 1;
    int column = (cupIndex % 2 == 0) ? 0 : 1;

    float x = (in_vertexCoords[0] + column) * widthLeap;
    float y = (in_vertexCoords[1] + row) * heightLeap;

    if (isPlate == 1){
        x += PLATE_WIDTH_START;
        y += PLATE_HEIGHT_START;
    }
    
    vertexCoords = vec2(x, y);
}