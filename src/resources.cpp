#include "resources.h"

#include "utils.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Resources::Resources()
{
   initShaderProgram(colorUniform, "shaders/colorUniform.vs.glsl", "shaders/colorUniform.fs.glsl");
    //mvp location get le uniform , regarder l'ancien tp pour le setup des ressources
    mvpLocationColorUniform = colorUniform.getUniformLoc("mat");
    colorLocationColorUniform = colorUniform.getUniformLoc("color");
}

