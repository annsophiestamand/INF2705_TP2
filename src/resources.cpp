#include "resources.h"

#include "utils.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Resources::Resources()
{
    initShaderProgram(colorUniform, "shaders/colorUniform.vs.glsl", "shaders/colorUniform.fs.glsl");
    mvpLocationColorUniform = colorUniform.getUniformLoc("mat");
    colorLocationColorUniform = colorUniform.getUniformLoc("color");

    initShaderProgram(texture, "shaders/texture.vs.glsl", "shaders/texture.fs.glsl");
    mvpLocationCup = texture.getUniformLoc("mvp");

    initShaderProgram(cup, "shaders/cup.vs.glsl", "shaders/texture.fs.glsl");
    mvpLocationCup = cup.getUniformLoc("mvp");
    textureIndexLocationCup = cup.getUniformLoc("cupIndex");
    isPlateLocationCup = cup.getUniformLoc("isPlate");
}

