#include "scene_attraction.h"

#include <iostream>
#include "imgui/imgui.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "utils.h"

const int FIRST_PERSON_VIEW = 0;
const int THIRD_PERSON_VIEW = 1;
const int MONKEY_VIEW = 2;

const GLfloat groundData[] = {
    -45.f, 0.0f, -45.f,  -3.0f, -3.0f,
     45.f, 0.0f, -45.f,  3.0f, -3.0f,
    -45.f, 0.0f,  45.f,  -3.0f, 3.0f,
     45.f, 0.0f,  45.f,  3.0f, 3.0f,
};

const GLubyte indexes[] = {
    2, 3, 0,
    3, 1, 0
};
SceneAttraction::SceneAttraction(Resources& res, bool& isMouseMotionEnabled)
: Scene(res)
, m_isMouseMotionEnabled(isMouseMotionEnabled)
, m_cameraMode(FIRST_PERSON_VIEW)
, m_isOrtho(false)
, m_cameraPosition(0.0f, 5.0f, 0.0f)
, m_cameraOrientation(0.0f, 0.0f)

, m_groundBuffer(GL_ARRAY_BUFFER, sizeof(groundData), groundData, GL_STATIC_DRAW)
, m_groundIndicesBuffer(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexes), indexes, GL_STATIC_DRAW)
, m_groundVao()
, m_groundDraw(m_groundVao, 6)

, m_largePlatformAngle(0.0f)
, m_smallPlatformAngle{0.0f}
, m_cupsAngles{0.0f}

, m_suzanne("../models/suzanne.obj")
, m_cup("../models/cup.obj")
, m_cupPlate("../models/plate.obj")
, m_smallPlatform("../models/smallPlatform.obj")
, m_largePlatform("../models/largePlatform.obj")
, m_cube("../models/cube.obj")

, m_groundTexture("../textures/grassSeamless.jpg")
, m_suzanneTexture("../textures/suzanneTextureShade.png")
, m_cupTextureAtlas("../textures/cupTextureAtlas.png")
, m_smallPlatformTexture("../textures/smallPlatform.png")
, m_largePlatformTexture("../textures/largePlatform.png")
{
    m_groundVao.bind();
    m_groundIndicesBuffer.bind();
    m_groundVao.specifyAttribute(m_groundBuffer, 0, 3, 5, 0); 
    m_groundVao.specifyAttribute(m_groundBuffer, 1, 2, 5, 3); 
    m_groundVao.unbind();
    
    m_groundTexture.setWrap(GL_REPEAT);
    m_groundTexture.enableMipmap();

    m_suzanneTexture.setFiltering(GL_LINEAR);
    m_suzanneTexture.setWrap(GL_CLAMP_TO_EDGE);

    m_cupTextureAtlas.setFiltering(GL_LINEAR);
    m_cupTextureAtlas.setWrap(GL_CLAMP_TO_EDGE);

    m_smallPlatformTexture.setFiltering(GL_LINEAR);
    m_smallPlatformTexture.setWrap(GL_CLAMP_TO_EDGE);

    m_largePlatformTexture.setFiltering(GL_NEAREST);
    m_largePlatformTexture.setWrap(GL_CLAMP_TO_EDGE);
}

SceneAttraction::~SceneAttraction() {}

void SceneAttraction::run(Window& w, double dt)
{
    const int CUP_QUANTITY = 4;
    const int PLATE_QUANTITY = 3;
    const int PLATEFORM_QUANTITY = 3;

    const float CUBE_COLORS[4][3] = {
        {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 1.0f},
        {1.0f, 1.0f, 0.0f}
    };

    ImGui::Begin("Scene Parameters");
    ImGui::Combo("Camera mode", &m_cameraMode, CAMERA_MODE_NAMES, N_CAMERA_MODE_NAMES);
    ImGui::Checkbox("Orthographic camera?", &m_isOrtho);
    ImGui::End();
    if (m_cameraMode != THIRD_PERSON_VIEW)
        updateInput(w, dt);

    // animation
    m_largePlatformAngle += 0.5 * dt;
    for (int i = 0; i < PLATEFORM_QUANTITY; i++) {
        m_smallPlatformAngle[i] += 0.5 * dt;
        for (int j = 0; j < CUP_QUANTITY; j++) {
            m_cupsAngles[i][j] += (0.5 + j * 0.5f) * dt;
        }
    }

    glm::mat4 proj = getProjectionMatrix(w);
    glm::mat4 view;
    if (m_cameraMode == FIRST_PERSON_VIEW || m_cameraMode == MONKEY_VIEW) {
        view = getCameraFirstPerson();
    } else {
        view = getCameraThirdPerson();
    }

    // sol
    {
        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.1f, 0.0f));
        glm::mat4 mvp = proj * view * model;
        m_resources.texture.use();
        m_groundTexture.use();
        glUniformMatrix4fv(m_resources.mvpLocationTexture, 1, GL_FALSE, &mvp[0][0]);
        m_groundDraw.draw();
    }

    // assiette centrale
    glm::mat4 modelPlateforme = glm::rotate(
        glm::mat4(1.0f),
        m_largePlatformAngle,
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
    {
        glm::mat4 mvp = proj * view * modelPlateforme;

        m_resources.texture.use();
        m_largePlatformTexture.use();
        glUniformMatrix4fv(m_resources.mvpLocationTexture, 1, GL_FALSE, &mvp[0][0]);
        m_largePlatform.draw();
    }

    // petites plateformes
    {
        m_resources.texture.use();
        m_smallPlatformTexture.use();

        for (int i = 0; i < PLATEFORM_QUANTITY; i++) {
            float angle = i * glm::radians(120.0f);
            glm::mat4 groupModel = modelPlateforme;
            groupModel = glm::rotate(groupModel, angle, glm::vec3(0, 1, 0));
            groupModel = glm::translate(groupModel, glm::vec3(15.0f, 0.0f, 0.0f));
            groupModel = glm::rotate(groupModel, m_smallPlatformAngle[i], glm::vec3(0,1,0));

            glm::mat4 mvpGroup = proj * view * groupModel;
            glUniformMatrix4fv(m_resources.mvpLocationTexture, 1, GL_FALSE, &mvpGroup[0][0]);
            m_smallPlatform.draw();
        }
    }

    // tasse et assiettes
    m_resources.cup.use();
    m_cupTextureAtlas.use();

    glm::mat4 suzanneModelMatrix(0.0f);
    glm::vec3 suzannePos(0.0f);
    float suzanneHeading = 0.0f;

    for (int i = 0; i < PLATE_QUANTITY; i++) {
        float angle = i * glm::radians(120.0f);
        glm::mat4 groupModel = modelPlateforme;
        groupModel = glm::rotate(groupModel, angle, glm::vec3(0,1,0));
        groupModel = glm::translate(groupModel, glm::vec3(15.0f, 0.5f, 0.0f));
        groupModel = glm::rotate(groupModel, m_smallPlatformAngle[i], glm::vec3(0,1,0));

        for (int j = 0; j < CUP_QUANTITY; j++) {
            float cupAngle = j * glm::radians(90.0f);
            glm::mat4 cupBase = groupModel;
            cupBase = glm::rotate(cupBase, cupAngle, glm::vec3(0,1,0));
            cupBase = glm::translate(cupBase, glm::vec3(6.0f, 0.0f, 0.0f));
            cupBase = glm::rotate(cupBase, m_cupsAngles[i][j], glm::vec3(0,1,0));

            // blo assiette
            {
                glm::mat4 plateModel = cupBase;
                glm::mat4 mvpPlate = proj * view * plateModel;
                glUniformMatrix4fv(m_resources.mvpLocationCup, 1, GL_FALSE, &mvpPlate[0][0]);
                glUniform1i(m_resources.isPlateLocationCup, 1);
                glUniform1i(m_resources.textureIndexLocationCup, j);
                m_cupPlate.draw();
            }
            // bloc tasse
            {
                glm::mat4 finalCupModel = glm::translate(cupBase, glm::vec3(0.0f, 0.12f, 0.0f));
                glm::mat4 mvpCup = proj * view * finalCupModel;
                glUniformMatrix4fv(m_resources.mvpLocationCup, 1, GL_FALSE, &mvpCup[0][0]);
                glUniform1i(m_resources.isPlateLocationCup, 0);
                glUniform1i(m_resources.textureIndexLocationCup, j);
                m_cup.draw();

                if (i == 0 && j == 0) {
                    glm::mat4 monkeyModel = glm::scale(finalCupModel, glm::vec3(2.0f));
                    suzanneModelMatrix = monkeyModel;
                    suzannePos = glm::vec3(monkeyModel[3]);
                    suzanneHeading = std::atan2(monkeyModel[2].x, monkeyModel[0].x);
                }
            }
        }
    }

    if (suzanneModelMatrix != glm::mat4(0.0f)) {
        m_resources.texture.use();
        glm::mat4 mvpSuzanne = proj * view * suzanneModelMatrix;
        glUniformMatrix4fv(m_resources.mvpLocationTexture, 1, GL_FALSE, &mvpSuzanne[0][0]);
        m_suzanneTexture.use();
        m_suzanne.draw();

        if (m_cameraMode == 2) {
            m_cameraPosition = suzannePos;
            m_cameraPosition.y = 3.8f;
            m_cameraOrientation.y = suzanneHeading;
        }
    }

    // === DESSIN DES CUBES ===
    m_resources.colorUniform.use();

    glm::vec3 cubePositions[4] = {
        glm::vec3(30.f, 3.0f, 0.0f),
        glm::vec3(-30.f, 3.0f, 0.0f),
        glm::vec3( 0.0f, 3.0f, 30.f),
        glm::vec3( 0.0f, 3.0f,-30.f)
    };

    for (int i = 0; i < 4; i++) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, cubePositions[i]);
        model = glm::scale(model, glm::vec3(6.f));
        glm::mat4 mvpCube = proj * view * model;

        glUniformMatrix4fv(m_resources.mvpLocationColorUniform, 1, GL_FALSE, &mvpCube[0][0]);
        glUniform3f(m_resources.colorLocationColorUniform, CUBE_COLORS[i][0], CUBE_COLORS[i][1], CUBE_COLORS[i][2]);
        m_cube.draw();
    }
}


void SceneAttraction::updateInput(Window& w, double dt)
{
    // Mouse input
    int x = 0, y = 0;
    if (m_isMouseMotionEnabled)
        w.getMouseMotion(x, y);
    const float MOUSE_SENSITIVITY = 0.1;
    float cameraMouvementX = y * MOUSE_SENSITIVITY;
    float cameraMouvementY = x * MOUSE_SENSITIVITY;;
    
    const float KEYBOARD_MOUSE_SENSITIVITY = 1.5f;
    if (w.getKeyHold(Window::Key::UP))
        cameraMouvementX -= KEYBOARD_MOUSE_SENSITIVITY;
    if (w.getKeyHold(Window::Key::DOWN))
        cameraMouvementX += KEYBOARD_MOUSE_SENSITIVITY;
    if (w.getKeyHold(Window::Key::LEFT))
        cameraMouvementY -= KEYBOARD_MOUSE_SENSITIVITY;
    if (w.getKeyHold(Window::Key::RIGHT))
        cameraMouvementY += KEYBOARD_MOUSE_SENSITIVITY;
    
    m_cameraOrientation.y -= cameraMouvementY * dt;
    m_cameraOrientation.x -= cameraMouvementX * dt;

    // Keyboard input
    glm::vec3 positionOffset = glm::vec3(0.0);
    const float SPEED = 10.f;
    if (w.getKeyHold(Window::Key::W))
        positionOffset.z -= SPEED;
    if (w.getKeyHold(Window::Key::S))
        positionOffset.z += SPEED;
    if (w.getKeyHold(Window::Key::A))
        positionOffset.x -= SPEED;
    if (w.getKeyHold(Window::Key::D))
        positionOffset.x += SPEED;
        
    if (w.getKeyHold(Window::Key::Q))
        positionOffset.y -= SPEED;
    if (w.getKeyHold(Window::Key::E))
        positionOffset.y += SPEED;

    positionOffset = glm::rotate(glm::mat4(1.0f), m_cameraOrientation.y, glm::vec3(0.0, 1.0, 0.0)) * glm::vec4(positionOffset, 1);
    m_cameraPosition += positionOffset * glm::vec3(dt);
}

glm::mat4 SceneAttraction::getCameraFirstPerson()
{
    glm::mat4 firstPerson = glm::mat4(1.0f);
    firstPerson = glm::translate(firstPerson, m_cameraPosition);

    glm::vec3 verticalOrientation = glm::vec3(0.0f,1.0f,0.0f);
    firstPerson = glm::rotate(firstPerson, m_cameraOrientation[1], verticalOrientation );

    glm::vec3 horizontalOrientation = glm::vec3(1.0f,0.0f,0.0f);
    firstPerson = glm::rotate(firstPerson, m_cameraOrientation[0], horizontalOrientation );

    return glm::inverse(firstPerson);
}


glm::mat4 SceneAttraction::getCameraThirdPerson()
{
    // la position de la cmaera doit toujorus regarder le (0,0,,0 ) soit le centre de la scene
    glm::vec3 target = glm::vec3(0,0,0);
    
    //definition des parametres d<offset pour la camera a la troisieme personne
    const float distance = 36.0f;
    const float height = 15.0f;

    glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), m_cameraOrientation[1], glm::vec3(0,1,0));
    glm::vec3 foward = glm::vec3(rotationMatrix * glm::vec4(0,0,-1,0));

    glm::vec3 cameraPos = target - foward*distance + glm::vec3(0,height, 0);

    return glm::lookAt(cameraPos, target, glm::vec3(0,1,0));
}

glm::mat4 SceneAttraction::getProjectionMatrix(Window& w)
{
    float aspectRatio = static_cast<float>(w.getWidth()) / static_cast<float>(w.getHeight());
    glm::mat4 proj;
    if (m_isOrtho)
        proj = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 300.0f);
    else
        proj = glm::perspective( glm::radians(70.0f), aspectRatio, 0.1f, 300.0f );

    return proj;
}

