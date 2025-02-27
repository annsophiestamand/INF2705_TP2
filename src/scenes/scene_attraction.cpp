#include "scene_attraction.h"

#include <iostream>
#include "imgui/imgui.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "utils.h"

// TODO - coordonnées de texture
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
, m_cameraMode(0)
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
    // TODO - spécifier les attributs

    m_groundVao.bind();
    m_groundIndicesBuffer.bind();
    m_groundVao.specifyAttribute(m_groundBuffer, 0, 3, 5, 0); 
    m_groundVao.specifyAttribute(m_groundBuffer, 1, 2, 5, 3); 
    m_groundVao.unbind();
    
    // TODO - init des textures
    m_groundTexture.setWrap(GL_REPEAT); // repeter la texture
    m_groundTexture.enableMipmap();

    m_suzanneTexture.setFiltering(GL_CLAMP_TO_EDGE);
    m_suzanneTexture.setWrap(GL_REPEAT);

    m_cupTextureAtlas.setFiltering(GL_CLAMP_TO_EDGE);
    m_cupTextureAtlas.setWrap(GL_REPEAT);

    m_smallPlatformTexture.setFiltering(GL_CLAMP_TO_EDGE);
    m_smallPlatformTexture.setWrap(GL_REPEAT);

    m_largePlatformTexture.setFiltering(GL_CLAMP_TO_EDGE);
    m_largePlatformTexture.setWrap(GL_NEAREST); // effet pixelise
}

SceneAttraction::~SceneAttraction()
{
}
//IMPORTANT : les rotations de tout objet dans le monde local comprends les axes aussi , une rotation dans l'axe de y changera l'orientation de x et z!!
void SceneAttraction::run(Window& w, double dt)
{    
    ImGui::Begin("Scene Parameters");
    ImGui::Combo("Camera mode", &m_cameraMode, CAMERA_MODE_NAMES, N_CAMERA_MODE_NAMES);
    ImGui::Checkbox("Orthographic camera?", &m_isOrtho);
    ImGui::End();
    
    updateInput(w, dt);    
    
    m_largePlatformAngle += 0.5 * dt;
    for (int i = 0; i < 3; i++)
    {
        m_smallPlatformAngle[i] += 0.5 * dt;
        for (int j = 0; j < 4; j++)
            m_cupsAngles[i][j] += (0.5 + j * 0.5f) * dt;
    }

    glm::mat4 model, proj, view, mvp;
    
    proj = getProjectionMatrix(w);
    
    if (m_cameraMode == 0 || m_cameraMode == 2)
        view = getCameraFirstPerson();
    else
        view = getCameraThirdPerson();
    // 1. dessin de la scene
    //  - Sol de taille 90 aux arretes   en y = -0.1 selon le doc
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, -0.1f, 0.0f)); // juste un petit decalage en bas
    mvp = proj * view * model;

    // texture sur le sol
    m_resources.texture.use();
    m_groundVao.bind();
    glUniformMatrix4fv(m_resources.mvpLocationTexture, 1, GL_FALSE, &mvp[0][0]);
    m_groundIndicesBuffer.bind();
    m_groundTexture.use();
    m_groundDraw.draw();

   //faire les cubes
    glm::vec3 cubePositions[4] = {
        glm::vec3( 30.f, 3.f,  0.f),
        glm::vec3(-30.f, 3.f,  0.f),
        glm::vec3(  0.f, 3.f, 30.f),
        glm::vec3(  0.f, 3.f,-30.f)
    };

    for (int i = 0; i < 4; i++)
    {
        //reset matrice model
        model = glm::mat4(1.0f);
        //effctuer une transaltion sde la position sur le model
        model = glm::translate(model, cubePositions[i]);
        //recalculer le mvp pour le cube en focntion des autres matrices
        mvp = proj * view * model;


        glUniformMatrix4fv(m_resources.mvpLocationColorUniform, 1, GL_FALSE, &mvp[0][0]);
        glUniform3f(m_resources.colorLocationColorUniform, 0.0f, 0.0f, 0.0f); // mettre les couleurs en noir
        m_cube.draw();
    }

  
    // 3)grande plateforme centrale
    //  une animationn de rotation autour de y = m_largePlatformAngle
   // matrice de base
    model = glm::mat4(1.0f);
    //prendre l'angle de rotation donne dans la classe et l,affecter au model
    model = glm::rotate(model, m_largePlatformAngle, glm::vec3(0.0f, 1.0f, 0.0f));
    
    mvp = proj * view * model;
    glUniformMatrix4fv(m_resources.mvpLocationColorUniform, 1, GL_FALSE, &mvp[0][0]);
    glUniform3f(m_resources.colorLocationColorUniform, 0.5f, 1.0f, 0.5f);
    m_largePlatform.draw();

    // 4 les petites plateormes (3 groupes)
    ///chacune placee sur un cercle de rayon 15,placer sur un cercle sperarer par 120 degrees
    //  les angles des mouvements temps reels : m_smallPlatformAngle[i]
    //  - Y = 0.5 legerement surevelee par rapport au sol
    for (int i = 0; i < 3; i++)
    {
        float angle = i * glm::radians(120.0f); // 120 degrees en radians, recalcul selon la plateforme 0, 120 240
        glm::mat4 groupModel(1.0f);

        // on tourne de base angle d el<objet alors qu<il est sur lorigine, cest la rotation sur lui meme les axes x et z changent
        groupModel = glm::rotate(groupModel, angle, glm::vec3(0,1,0));

        // on translate a (15,0.5,0) . le rayon de 15 unites dans la direction oriente par le radiant
        groupModel = glm::translate(groupModel, glm::vec3(15.0f, 0.5f, 0.0f));

        // on applique la rotation de l'animation, une rotation par raport a un cercle de sitance de 15 a loriginie
        groupModel = glm::rotate(groupModel, m_smallPlatformAngle[i], glm::vec3(0,1,0));

        // Dessin de la petite plateforme
        mvp = proj * view * groupModel; //faire le calcul final pour affecter sur l<objet
        glUniformMatrix4fv(m_resources.mvpLocationColorUniform, 1, GL_FALSE, &mvp[0][0]);
        glUniform3f(m_resources.colorLocationColorUniform, 1.0f, 0.5f, 0.5f); 
        m_smallPlatform.draw();


        // 5. tasses et petites assiettes, 4 par groupes, un set de tasses pour chq plateforme
        //  chacune sur un cercle de rayon 6 autour de la petite plateforme
        // separees par 90 degree (360/4 = 90)
        //  chaque tasses a son propres angle d<animation qui va determiner 
        //  assiette est à y=0 (relative), la tasse à y=0.12 par dessus
        for (int j = 0; j < 4; j++)
        {
            //orientation de base pour les 4 tasses
            float cupangle = j * glm::radians(90.0f);

            // On part du groupModel (parent), et on ajoute la position/rotation de la tasse
            glm::mat4 cupBase = groupModel;
            cupBase = glm::rotate(cupBase, cupangle, glm::vec3(0,1,0));
            cupBase = glm::translate(cupBase, glm::vec3(6.0f, 0.0f, 0.0f));
            cupBase = glm::rotate(cupBase, m_cupsAngles[i][j], glm::vec3(0,1,0));

            // petites assiette
            {
                glm::mat4 plateModel = cupBase;
                // plateModel = glm::translate(plateModel, glm::vec3(0.0f, 0.0f, 0.0f)); // déjà sur y=0
                mvp = proj * view * plateModel;
                glUniformMatrix4fv(m_resources.mvpLocationColorUniform, 1, GL_FALSE, &mvp[0][0]);
                glUniform3f(m_resources.colorLocationColorUniform, 0.8f, 0.8f, 0.8f); // gris
                m_cupPlate.draw();
            }

            // tasse
            {
                glm::mat4 finalCupModel = cupBase;
                finalCupModel = glm::translate(finalCupModel, glm::vec3(0.0f, 0.12f, 0.0f)); // tasse un peu plus haut
                mvp = proj * view * finalCupModel;
                glUniformMatrix4fv(m_resources.mvpLocationColorUniform, 1, GL_FALSE, &mvp[0][0]);
                glUniform3f(m_resources.colorLocationColorUniform, 0.2f, 0.6f, 1.0f); // bleu
                m_cup.draw();

                if (i == 0 && j == 0)
                {
                    glm::mat4 monkeyModel = finalCupModel;
                    monkeyModel = glm::scale(monkeyModel, glm::vec3(2.0f));
  
                    glm::vec3 monkeyPos = glm::vec3(monkeyModel[3]); // x,y,z
                    float monkeyHeading = std::atan2(monkeyModel[2].x, monkeyModel[0].x);

                    
                    mvp = proj * view * monkeyModel;
                    glUniformMatrix4fv(m_resources.mvpLocationColorUniform, 1, GL_FALSE, &mvp[0][0]);
                    glUniform3f(m_resources.colorLocationColorUniform, 1.0f, 0.7f, 0.0f); // orange
                    m_suzanne.draw();

                   
                    if (m_cameraMode == 2)
                    {
                        m_cameraPosition = monkeyPos;
                        m_cameraPosition.y = 3.8f; 
                        m_cameraOrientation.y = monkeyHeading;
                    }
                }
            }
        }
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
    // TODO
    // message discord annso.
    //CONTEXTE : les attributs de la classe m_cameraPostion et m_cameraOrientation stockent les positions enregistrer relle de la camera
    // nous voulons creer une matrice pour bind la camera a ces positions pour simuler l'effet de premiere personne. Normalement je suppsoe
    // que, a chaque frame nous reclaculons avec les entree du clavienr pour faire une matrice qui va deplacer la postiion de la scene pour 
    //correspondre avec la postion theoriqeu de la camera

    //1.commencer avec une matrice identite creer
    glm::mat4 firstPerson = glm::mat4(1.0f);

    //2.appliquer un deplacement de la camera chacuns des point de la camera :
    //exemple: nous avons precedemment deplacer la camera or nous voulons creer une matrice qui
    // va depalcer les points vers la position actuelle pour qu<on deplace vers la position que devrait etre la camera
    firstPerson = glm::translate(firstPerson, m_cameraPosition);

    //3. pivoter la matrice selon l<axe vertical,pour oreinter la camera
    glm::vec3 verticalOrientation = glm::vec3(0.0f,1.0f,0.0f);
    firstPerson = glm::rotate(firstPerson, m_cameraOrientation[1], verticalOrientation );

    //4. oreintation horizontale, orienter vers la camera
    glm::vec3 horizontalOrientation = glm::vec3(1.0f,0.0f,0.0f);
    firstPerson = glm::rotate(firstPerson, m_cameraOrientation[0], horizontalOrientation );

    //5.pour obtenir la matrice de vue relle il faut tout inverser transformation de la camera, affecter tous les points a ceci

    return glm::inverse(firstPerson);
}


glm::mat4 SceneAttraction::getCameraThirdPerson()
{
    // TODO
    return glm::mat4(1.0);
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

