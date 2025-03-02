#include "lab_m1/Tema2/Tema2.h"

#include <vector>
#include <string>
#include <iostream>
#include <glm/gtx/vector_angle.hpp>
#include <vector>

using namespace std;


/*
 *  To find out more about `FrameStart`, `Update`, `FrameEnd`
 *  and the order in which they are called, see `world.cpp`.
 */


DroneGame::DroneGame()
{
}


DroneGame::~DroneGame()
{
}

Mesh* CreateSquareTex(
    const std::string& name,
    glm::vec3 leftBottomCorner,
    float length,
    glm::vec3 color,
    bool fill)
{
    glm::vec3 corner = leftBottomCorner;
    glm::vec3 normal = glm::vec3(0, 1, 0);
    std::vector<VertexFormat> vertices =
    {
        VertexFormat(corner, color, normal, glm::vec2(0.0f, 0.0f)),
        VertexFormat(corner + glm::vec3(length, 0, 0), color, normal, glm::vec2(1.0f, 0.0f)),
        VertexFormat(corner + glm::vec3(length, length, 0), color, normal, glm::vec2(1.0f, 1.0f)),
        VertexFormat(corner + glm::vec3(0, length, 0), color, normal, glm::vec2(0.0f, 1.0f))
    };

    Mesh* square = new Mesh(name);
    std::vector<unsigned int> indices = { 0, 1, 2, 3 };

    if (!fill) {
        square->SetDrawMode(GL_POINTS);
    }
    else {
        indices.push_back(0);
        indices.push_back(2);
    }

    square->InitFromData(vertices, indices);
    return square;
}


void DroneGame::Init()
{
    camera = new implemented::Camera();
    camera->Set(glm::vec3(0, 2, 3.5f), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0));
    cameraDown = new implemented::Camera();
    cameraDown->Set(glm::vec3(0, 2, 3.5f), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0));
    cameraDown->RotateFirstPerson_OX(-0.7);
    cameraFrameBuffer = new implemented::Camera();
    cameraFrameBuffer->Set(glm::vec3(0, 2, 3.5f), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0));
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    {
        Mesh* mesh = new Mesh("box");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "box.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }

    {
        Mesh* mesh = new Mesh("sphere");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "sphere.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }

    {
        glm::vec3 color = glm::vec3(0.0f, 0.0f, 0.5f);
        std::vector<VertexFormat> vertices =
        {
            VertexFormat(glm::vec3(0,0,0) + glm::vec3(0, 0, 0), color),
            VertexFormat(glm::vec3(0,0,0) + glm::vec3(1, 0, 0), color),
            VertexFormat(glm::vec3(0,0,0) + glm::vec3(2, 0, 0), color),
            VertexFormat(glm::vec3(0,0,0) + glm::vec3(3, 0, 0), color),
            VertexFormat(glm::vec3(0,0,0) + glm::vec3(4, 0, 0), color),
            VertexFormat(glm::vec3(0,0,0) + glm::vec3(5, 0, 0), color),

            VertexFormat(glm::vec3(0,0,0) + glm::vec3(0, 0, 1), color),
            VertexFormat(glm::vec3(0,0,0) + glm::vec3(1, 0, 1), color),
            VertexFormat(glm::vec3(0,0,0) + glm::vec3(2, 0, 1), color),
            VertexFormat(glm::vec3(0,0,0) + glm::vec3(3, 0, 1), color),
            VertexFormat(glm::vec3(0,0,0) + glm::vec3(4, 0, 1), color),
            VertexFormat(glm::vec3(0,0,0) + glm::vec3(5, 0, 1), color),

            VertexFormat(glm::vec3(0,0,0) + glm::vec3(0, 0, 2), color),
            VertexFormat(glm::vec3(0,0,0) + glm::vec3(1, 0, 2), color),
            VertexFormat(glm::vec3(0,0,0) + glm::vec3(2, 0, 2), color),
            VertexFormat(glm::vec3(0,0,0) + glm::vec3(3, 0, 2), color),
            VertexFormat(glm::vec3(0,0,0) + glm::vec3(4, 0, 2), color),
            VertexFormat(glm::vec3(0,0,0) + glm::vec3(5, 0, 2), color),

            VertexFormat(glm::vec3(0,0,0) + glm::vec3(0, 0, 3), color),
            VertexFormat(glm::vec3(0,0,0) + glm::vec3(1, 0, 3), color),
            VertexFormat(glm::vec3(0,0,0) + glm::vec3(2, 0, 3), color),
            VertexFormat(glm::vec3(0,0,0) + glm::vec3(3, 0, 3), color),
            VertexFormat(glm::vec3(0,0,0) + glm::vec3(4, 0, 3), color),
            VertexFormat(glm::vec3(0,0,0) + glm::vec3(5, 0, 3), color),

            VertexFormat(glm::vec3(0,0,0) + glm::vec3(0, 0, 4), color),
            VertexFormat(glm::vec3(0,0,0) + glm::vec3(1, 0, 4), color),
            VertexFormat(glm::vec3(0,0,0) + glm::vec3(2, 0, 4), color),
            VertexFormat(glm::vec3(0,0,0) + glm::vec3(3, 0, 4), color),
            VertexFormat(glm::vec3(0,0,0) + glm::vec3(4, 0, 4), color),
            VertexFormat(glm::vec3(0,0,0) + glm::vec3(5, 0, 4), color),

            VertexFormat(glm::vec3(0,0,0) + glm::vec3(0, 0, 5), color),
            VertexFormat(glm::vec3(0,0,0) + glm::vec3(1, 0, 5), color),
            VertexFormat(glm::vec3(0,0,0) + glm::vec3(2, 0, 5), color),
            VertexFormat(glm::vec3(0,0,0) + glm::vec3(3, 0, 5), color),
            VertexFormat(glm::vec3(0,0,0) + glm::vec3(4, 0, 5), color),
            VertexFormat(glm::vec3(0,0,0) + glm::vec3(5, 0, 5), color), 
        };

        std::vector<unsigned int> indices = {
            0, 1, 6,
            6, 7, 1,

            1, 2, 7,
            8, 7, 2,

            2, 3, 8,
            9, 8, 3,

            3, 4, 9,
            10, 9, 4,

            4, 5, 10,
            11, 10, 5,
            //
            6, 7, 12,
            13, 12, 7,

            7, 8, 13,
            14, 13, 8,

            8, 9, 14,
            15, 14, 9,

            9, 10, 15,
            16, 15, 10,
            
            10, 11, 16,
            17, 16, 11,
            ///
            12, 13, 18,
            19, 18, 13,

            13, 14, 19,
            20, 19, 14,

            14, 15, 20,
            21, 20, 15,

            15, 16, 21,
            22, 21, 16,

            16, 17, 22,
            23, 22, 17,
            ///
            18, 19, 24,
            25, 24, 19,

            19, 20, 25,
            26, 25, 20,

            20, 21, 26,
            27, 26, 21,

            21, 22, 27,
            28, 27, 22,

            22, 23, 28,
            29, 28, 23,
            ///
            24, 25, 30,
            31, 30, 25,

            25, 26, 31,
            32, 31, 26,

            26, 27, 32,
            33, 32, 27,

            27, 28, 33,
            34, 33, 28,

            28, 29, 34,
            35, 34, 29,
        };

        meshes["plane"] = new Mesh("plane");
        meshes["plane"]->SetDrawMode(GL_TRIANGLES);

        meshes["plane"]->InitFromData(vertices, indices);
    }

    {
        Shader* shader = new Shader("ColorShader");
        shader->AddShader(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::SHADERS, "MVP.Texture.tema2.VS.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::SHADERS, "colorShader.FS.glsl"), GL_FRAGMENT_SHADER);
        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }

    {
        Shader* shader = new Shader("PlaneShader");
        shader->AddShader(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::SHADERS, "PlaneShader.VS.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::SHADERS, "PlaneShader.FS.glsl"), GL_FRAGMENT_SHADER);
        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }

    {
        Shader* shader = new Shader("FrameBufferShader");
        shader->AddShader(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::SHADERS, "MVP.Texture.VS.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::SHADERS, "Screen.FS.glsl"), GL_FRAGMENT_SHADER);
        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }

    glm::vec2 resolution = window->GetResolution();

    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, resolution.x, resolution.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, resolution.x, resolution.y);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    frameBufferMesh = CreateSquareTex("framebuffer", glm::vec3(0), 1, glm::vec3(1, 1, 1), true);
    

    for (int i = 0; i < 30; i++) {
        float x = rand() % 400 - 200;
        float z = rand() % 400 - 200;
        if (x < 5) x += 5;
        if (z < 5) z += 5;
        float scale = rand() % 2 + 1;
        obstacles.push_back(glm::vec3(x, 0, z));
        scaling.push_back(scale);
    }

    reload = glm::vec3(0.0f, 2.0f, 0.0f);
    enemy = glm::vec3(rand() % 200 - 100, 1, rand() % 200 - 100);
}


void DroneGame::FrameStart()
{
    glm::ivec2 resolution = window->GetResolution();
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, resolution.x, resolution.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, resolution.x, resolution.y);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glEnable(GL_DEPTH_TEST);

    glClearColor(0.5, 0.5, 0.9, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    resolution = window->GetResolution();

    glViewport(0, 0, resolution.x, resolution.y);
    cameraDown->position = camera->position - glm::vec3(0, 0 ,-0.5);
}

struct Box {
    int minX, minY, minZ, maxX, maxY, maxZ;

    Box(glm::vec3 pos, glm::vec3 size) {
        int halfWidth = size.x / 2;
        int halfHeight = size.y / 2;
        int halfDepth = size.z / 2;

        minX = pos.x - halfWidth;
        maxX = pos.x + halfWidth;

        minY = pos.y - halfHeight;
        maxY = pos.y + halfHeight;

        minZ = pos.z - halfDepth;
        maxZ = pos.z + halfDepth;
    }
};

bool isPointInsideAABB(glm::vec3 point, Box box) {
    return (
        point.x >= box.minX &&
        point.x <= box.maxX &&
        point.y >= box.minY &&
        point.y <= box.maxY &&
        point.z >= box.minZ &&
        point.z <= box.maxZ
        );
}

float calculateAngle(glm::vec3 start, glm::vec3 end) {

    glm::vec3 direction = glm::normalize(glm::vec3(end.x - start.x, 0.0f, end.z - start.z));

    glm::vec3 reference = glm::vec3(1.0f, 0.0f, 0.0f);
    float angleRadians = glm::atan(direction.z, direction.x);
    float angleDegrees = glm::degrees(angleRadians);

    return angleDegrees;
}


void DroneGame::Update(float deltaTimeSeconds)
{
    dt = deltaTimeSeconds;
    projectionMatrix = glm::perspective(RADIANS(FoV), window->props.aspectRatio, 0.01f, 200.0f);
    rotation = glm::degrees(glm::angle(glm::normalize(glm::vec3(camera->forward.x, 0.0, camera->forward.z)), glm::normalize(glm::vec3(0.0, 0.0, 1.0))));
    if (camera->forward.x > 0) {
        rotation = -(rotation - 180);
    }
    else {
        rotation = rotation + 180;
    }

    glm::vec3 pos;
    if (bullet == 0) {
        pos = reload;
    }
    else {
        pos = glm::vec3(-enemy.x, 0, -enemy.z);
    }
    glm::vec3 up = glm::normalize(glm::vec3(camera->position.x, 0.0f, camera->position.z));
    glm::vec3 forward = glm::normalize(glm::vec3(pos.x, 0.0f, pos.z));
    arrow_rot = 90 + 90 - calculateAngle(-camera->position, pos);

    if (window->KeyHold(GLFW_KEY_1)) {
        arrow_rot += deltaTimeSeconds * 20;
    }

    if (window->KeyHold(GLFW_KEY_2)) {
        arrow_rot -= deltaTimeSeconds * 20;
    }

    for (int i = 0; i < 30; i++) {
        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, obstacles[i] + glm::vec3(0.0f, 1.0f + scaling[i], 0.0f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f, 30.0f, 0.5f));

        RenderMesh(meshes["box"], shaders["ColorShader"], modelMatrix, camera, glm::vec3(0.9, 0.9, 0.9));

        glm::mat4 modelElice = glm::mat4(1);
        modelElice = glm::translate(modelElice, obstacles[i] + glm::vec3(0.0f, 1.0f + scaling[i] + 15.0f, 0.0f));
        modelElice = glm::rotate(modelElice, RADIANS((float)glfwGetTime() * 100), glm::vec3(1, 0, 0));
        modelElice = glm::rotate(modelElice, RADIANS(90.0f + obstacles[i].x), glm::vec3(1, 0, 0));
        modelElice = glm::scale(modelElice, glm::vec3(0.5f, 15.0f, 0.5f));

        RenderMesh(meshes["sphere"], shaders["ColorShader"], modelElice, camera, glm::vec3(0.9, 0.9, 0.9));

        float size = 5;
        if (isPointInsideAABB(camera->position, Box(obstacles[i] + glm::vec3(0.0f, 1.0f + scaling[i], 0.0f), glm::vec3(size * 0.5f, size * 6.5f, size * 0.5f)))) {
            //std::cout << "Coliziune" << std::endl;
            if (speedFrontMomentum > 0) {
                speedBackMomentum = speedFrontMomentum / 2;
                speedFrontMomentum = -1;
                collisionDirection = 0;
            }
            else if (speedBackMomentum > 0) {
                speedFrontMomentum = speedBackMomentum / 2;
                speedBackMomentum = -1;
                collisionDirection = 1;
            }
            if (speedLeftMomentum > 0) {
                speedRightMomentum = speedLeftMomentum / 2;
                speedLeftMomentum = -1;
                collisionDirection = 2;
            }
            else if (speedRightMomentum > 0) {
                speedLeftMomentum = speedRightMomentum / 2;
                speedRightMomentum = -1;
                collisionDirection = 3;
            }

            switch (collisionDirection)
            {
            case 0:
                speedBackMomentum = 1;
                break;
            case 1:
                speedFrontMomentum = 1;
                break;
            case 2:
                speedRightMomentum = 1;
                break;
            case 3:
                speedLeftMomentum = 1;
                break;
            default:
                break;
            }
        }
    }
    glm::mat4 modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, reload);

    RenderMesh(meshes["box"], shaders["ColorShader"], modelMatrix, camera, glm::vec3(0.5, 0.5, 0));
    if (isPointInsideAABB(camera->position, Box(reload, glm::vec3(2, 2, 2)))) {
        bullet = 5;
    }

    if (bomb.y > -2) {
        modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, bomb);
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5, 0.5, 0.5));

        RenderMesh(meshes["box"], shaders["ColorShader"], modelMatrix, camera, glm::vec3(0.5, 0.5, 0));
        bomb.y -= deltaTimeSeconds * 10;
    }

    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, enemy);

    RenderMesh(meshes["box"], shaders["ColorShader"], modelMatrix, camera, glm::vec3(0, 0.5, 0));
    if (isPointInsideAABB(bomb, Box(enemy, glm::vec3(2, 2, 2)))) {
        enemy.x = rand() % 200 - 100;
        enemy.z = rand() % 200 - 100;
        bomb.y = -2;
    }

    // std::cout << camera->position << std::endl;
    for (int i = 0; i <= 100; i++) {
        int x = i / 10 - 5 + camera->position.x / 50;
        int y = i % 10 - 5 + camera->position.z / 50;
        int sign = i % 2 == 0 ? -1 : 1;
        float ran = (rand() % 100) / 100;
        shaders["PlaneShader"]->Use();
        glUniform1f(shaders["PlaneShader"]->GetUniformLocation("uScale"), 100.0f);
        glUniform1f(shaders["PlaneShader"]->GetUniformLocation("uTime"), glfwGetTime());
        {
            glm::mat4 modelMatrix = glm::mat4(1);
            modelMatrix = glm::scale(modelMatrix, glm::vec3(10.0f, 1.0f, 10.0f));
            modelMatrix = glm::translate(modelMatrix, glm::vec3(5 * x, 0, 5 * y));
            modelMatrix = glm::translate(modelMatrix, glm::vec3(-2.5f, 0, -2.5f)); 

            RenderMesh(meshes["plane"], shaders["PlaneShader"], modelMatrix, camera, glm::vec3(0.5, 0.5, 0.5));
        }
    }

    {
        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, camera->position - glm::vec3(0.0f,0.4f,0.0f));
        modelMatrix = glm::rotate(modelMatrix, RADIANS(speedLeftMomentum * 10), glm::vec3(0, 0, 1));
        modelMatrix = glm::rotate(modelMatrix, RADIANS(-speedRightMomentum * 10), glm::vec3(0, 0, 1));
        modelMatrix = glm::rotate(modelMatrix, RADIANS(speedBackMomentum * 10), glm::vec3(1, 0, 0));
        modelMatrix = glm::rotate(modelMatrix, RADIANS(-speedFrontMomentum * 10), glm::vec3(1, 0, 0));
        modelMatrix = glm::rotate(modelMatrix, RADIANS(45.0f - rotation), glm::vec3(0, 1, 0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0f, 0.2f, 0.05f));
        RenderMesh(meshes["box"], shaders["ColorShader"], modelMatrix, camera, glm::vec3(0.5, 0.5, 0.5));
    }

    {
        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, camera->position - glm::vec3(0.0f, 0.4f, 0.0f));
        modelMatrix = glm::rotate(modelMatrix, RADIANS(speedLeftMomentum * 10), glm::vec3(0, 0, 1));
        modelMatrix = glm::rotate(modelMatrix, RADIANS(-speedRightMomentum * 10), glm::vec3(0, 0, 1));
        modelMatrix = glm::rotate(modelMatrix, RADIANS(speedBackMomentum * 10), glm::vec3(1, 0, 0));
        modelMatrix = glm::rotate(modelMatrix, RADIANS(-speedFrontMomentum * 10), glm::vec3(1, 0, 0));
        modelMatrix = glm::rotate(modelMatrix, RADIANS(135.0f - rotation), glm::vec3(0, 1, 0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0f, 0.2f, 0.05f));
        RenderMesh(meshes["box"], shaders["ColorShader"], modelMatrix, camera, glm::vec3(0.5, 0.5, 0.5));
    }

    {
        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, camera->position - glm::vec3(0.0, 0.1f, 0.0f));
        modelMatrix = glm::rotate(modelMatrix, RADIANS(speedLeftMomentum * 10), glm::vec3(0, 0, 1));
        modelMatrix = glm::rotate(modelMatrix, RADIANS(-speedRightMomentum * 10), glm::vec3(0, 0, 1));
        modelMatrix = glm::rotate(modelMatrix, RADIANS(speedBackMomentum * 10), glm::vec3(1, 0, 0));
        modelMatrix = glm::rotate(modelMatrix, RADIANS(-speedFrontMomentum * 10), glm::vec3(1, 0, 0));
        modelMatrix = glm::rotate(modelMatrix, RADIANS(-rotation), glm::vec3(0, 1, 0));
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0.12, 0.0f, -0.12));
        modelMatrix = glm::rotate(modelMatrix, RADIANS(float(glfwGetTime() * 1000)), glm::vec3(0, 1, 0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.15f / 2, 0.001f, 0.01f));
        RenderMesh(meshes["box"], shaders["ColorShader"], modelMatrix, camera, glm::vec3(1, 1, 1));
    }

    {
        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, camera->position - glm::vec3(0.0, 0.1f, 0.0f));
        modelMatrix = glm::rotate(modelMatrix, RADIANS(speedLeftMomentum * 10), glm::vec3(0, 0, 1));
        modelMatrix = glm::rotate(modelMatrix, RADIANS(-speedRightMomentum * 10), glm::vec3(0, 0, 1));
        modelMatrix = glm::rotate(modelMatrix, RADIANS(speedBackMomentum * 10), glm::vec3(1, 0, 0));
        modelMatrix = glm::rotate(modelMatrix, RADIANS(-speedFrontMomentum * 10), glm::vec3(1, 0, 0));
        modelMatrix = glm::rotate(modelMatrix, RADIANS(-rotation), glm::vec3(0, 1, 0));
        modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.12, 0.0f, -0.12));
        modelMatrix = glm::rotate(modelMatrix, RADIANS(float(glfwGetTime() * 1000)), glm::vec3(0, 1, 0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.15f / 2, 0.001f, 0.01f));
        RenderMesh(meshes["box"], shaders["ColorShader"], modelMatrix, camera, glm::vec3(1, 1, 1));
    }


    glm::vec3 col;
    if (bullet == 0) {
        col = glm::vec3(0.5, 0.5, 0);
    }
    else {
        col = glm::vec3(0, 0.5, 0);
    }
    {
        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, camera->position - glm::vec3(0.0, 0.1f, 0.0f));
        modelMatrix = glm::rotate(modelMatrix, RADIANS(-rotation), glm::vec3(0, 1, 0));
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 0.0f, -0.12));
        modelMatrix = glm::rotate(modelMatrix, RADIANS(rotation + arrow_rot), glm::vec3(0, 1, 0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.15f / 4, 0.001f, 0.01f / 2));
        RenderMesh(meshes["box"], shaders["ColorShader"], modelMatrix, camera, col);
    }

    {
        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, camera->position - glm::vec3(0.0, 0.1f, 0.0f));
        modelMatrix = glm::rotate(modelMatrix, RADIANS(-rotation), glm::vec3(0, 1, 0));
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 0.0f, -0.12));
        modelMatrix = glm::rotate(modelMatrix, RADIANS(rotation + arrow_rot), glm::vec3(0, 1, 0));
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0.04, 0.0f, 0.0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.15f / 2, 0.001f, 0.01f / 2));
        RenderMesh(meshes["sphere"], shaders["ColorShader"], modelMatrix, camera, col);
    }

    glm::ivec2 resolution = window->GetResolution();

    glViewport(resolution.x - resolution.x / 4, resolution.y - resolution.y / 4, resolution.x / 4, resolution.y / 4);

    glClearColor(0.0, 0.0, 0.0, 1.0);

    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, reload);

    RenderMesh(meshes["box"], shaders["ColorShader"], modelMatrix, cameraDown, glm::vec3(0.5, 0.5, 0));

    if (bomb.y > -2) {
        modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, bomb);
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5, 0.5, 0.5));

        RenderMesh(meshes["box"], shaders["ColorShader"], modelMatrix, cameraDown, glm::vec3(0.5, 0.5, 0));
        //bomb.y -= deltaTimeSeconds * 10;
    }

    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, enemy);

    RenderMesh(meshes["box"], shaders["ColorShader"], modelMatrix, cameraDown, glm::vec3(0, 0.5, 0));

    for (int i = 0; i < 30; i++) {
        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, obstacles[i] + glm::vec3(0.0f, 1.0f + scaling[i], 0.0f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f, 30.0f, 0.5f));

        RenderMesh(meshes["box"], shaders["ColorShader"], modelMatrix, cameraDown, glm::vec3(0.9, 0.9, 0.9));
    }

    for (int i = 0; i <= 100; i++) {
        int x = i / 10 - 5 + camera->position.x / 50;
        int y = i % 10 - 5 + camera->position.z / 50;
        int sign = i % 2 == 0 ? -1 : 1;
        float ran = (rand() % 100) / 100;
        shaders["PlaneShader"]->Use();
        glUniform1f(shaders["PlaneShader"]->GetUniformLocation("uScale"), 100.0f);
        glUniform1f(shaders["PlaneShader"]->GetUniformLocation("uTime"), glfwGetTime());
        {
            glm::mat4 modelMatrix = glm::mat4(1);
            modelMatrix = glm::scale(modelMatrix, glm::vec3(10.0f, 1.0f, 10.0f));
            modelMatrix = glm::translate(modelMatrix, glm::vec3(5 * x, 0, 5 * y));
            modelMatrix = glm::translate(modelMatrix, glm::vec3(-2.5f, 0, -2.5f));

            RenderMesh(meshes["plane"], shaders["PlaneShader"], modelMatrix, cameraDown, glm::vec3(0.5, 0.5, 0.5));
        }
    }

    /*std::cout << camera->forward << std::endl;
    std::cout << camera->right << std::endl;
    std::cout << camera->up << std::endl;*/
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glm::vec2 res = window->GetResolution();
    glViewport(0, 0, res.x, res.y);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-1.92f, -0.1f, 0));
    model = glm::scale(model, glm::vec3(1280 * 0.003, 720 * 0.003, 1.0f));

    RenderSimpleMesh(frameBufferMesh, shaders["FrameBufferShader"], model, textureColorbuffer);

    if (camera->position.y < 0.5) {
        glClearColor(0.0f, 0.0f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }
}

void DroneGame::RenderSimpleMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, unsigned int texture1)
{
    if (!mesh || !shader || !shader->GetProgramID())
        return;

    glUseProgram(shader->program);

    GLint loc_model_matrix = glGetUniformLocation(shader->program, "Model");
    glUniformMatrix4fv(loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    glm::mat4 viewMatrix = cameraFrameBuffer->GetViewMatrix();
    int loc_view_matrix = glGetUniformLocation(shader->program, "View");
    glUniformMatrix4fv(loc_view_matrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));

    float left = -window->props.aspectRatio;
    float right = window->props.aspectRatio;
    float bottom = -1.0f;
    float top = 1.0f;
    float nearPlane = 0.01f;
    float farPlane = 200.0f;

    glm::mat4 projectionMatrix = glm::ortho(left, right, bottom, top, nearPlane, farPlane);
    int loc_projection_matrix = glGetUniformLocation(shader->program, "Projection");
    glUniformMatrix4fv(loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    if (texture1)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        int loc_texture = glGetUniformLocation(shader->program, "texture1");
        glUniform1i(loc_texture, 0);
    }

    glBindVertexArray(mesh->GetBuffers()->m_VAO);
    glDrawElements(mesh->GetDrawMode(), static_cast<int>(mesh->indices.size()), GL_UNSIGNED_INT, 0);
}

void DroneGame::FrameEnd()
{
    // DrawCoordinateSystem(camera->GetViewMatrix(), projectionMatrix);
}


void DroneGame::RenderMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, implemented::Camera* cam, glm::vec3 color)
{
    if (!mesh || !shader || !shader->program)
        return;

    // Render an object using the specified shader and the specified position
    shader->Use();
    glUniform3f(shader->GetUniformLocation("color"), color.r, color.g, color.b);
    glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(cam->GetViewMatrix()));
    glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
    glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    mesh->Render();
}


/*
 *  These are callback functions. To find more about callbacks and
 *  how they behave, see `input_controller.h`.
 */


void DroneGame::OnInputUpdate(float deltaTime, int mods)
{
    if (window->KeyHold(GLFW_KEY_SPACE) && bomb.y < -1 && bullet > 0) {
        bomb = camera->position;
        bullet--;
    }
    if (deltaTime == INFINITY) return;
    // move the camera only if MOUSE_RIGHT button is pressed
        float cameraSpeed = 2.0f;
        speedFrontActivated = false;
        if (window->KeyHold(GLFW_KEY_W)) {
            // TODO(student): Translate the camera forward
            speedFrontMomentum += deltaTime;
            speedFrontActivated = true;
            FoV = glm::min(FoV + cameraSpeed * deltaTime * 4, 120.0f);
            // camera->MoveForward(cameraSpeed * deltaTime);
        }
        if (!speedFrontActivated) {
            speedFrontMomentum -= deltaTime;
        }
        speedFrontMomentum = glm::min(glm::max(speedFrontMomentum, 0.0f), 1.0f);
        speedFront = glm::mix(0.0f, 10.0f, speedFrontMomentum) * cameraSpeed;
        camera->MoveForward(speedFront * deltaTime);

        speedLeftActivated = false;
        if (window->KeyHold(GLFW_KEY_A)) {
            // TODO(student): Translate the camera forward
            speedLeftMomentum += deltaTime;
            speedLeftActivated = true;
            FoV = glm::min(FoV + cameraSpeed * deltaTime * 4, 120.0f);
            // camera->MoveForward(cameraSpeed * deltaTime);
        }
        if (!speedLeftActivated) {
            speedLeftMomentum -= deltaTime;
        }
        speedLeftMomentum = glm::min(glm::max(speedLeftMomentum, 0.0f), 1.0f);
        speedLeft = glm::mix(0.0f, 10.0f, speedLeftMomentum) * cameraSpeed;
        camera->TranslateRight(-speedLeft * deltaTime);

        speedBackActivated = false;
        if (window->KeyHold(GLFW_KEY_S)) {
            // TODO(student): Translate the camera forward
            speedBackMomentum += deltaTime;
            speedBackActivated = true;
            FoV = glm::min(FoV + cameraSpeed * deltaTime * 4, 120.0f);
            // camera->MoveForward(cameraSpeed * deltaTime);
        }
        if (!speedBackActivated) {
            speedBackMomentum -= deltaTime;
        }
        speedBackMomentum = glm::min(glm::max(speedBackMomentum, 0.0f), 1.0f);
        speedBack = glm::mix(0.0f, -10.0f, speedBackMomentum) * cameraSpeed;
        camera->MoveForward(speedBack * deltaTime);

        speedRightActivated = false;
        if (window->KeyHold(GLFW_KEY_D)) {
            // TODO(student): Translate the camera forward
            speedRightMomentum += deltaTime;
            speedRightActivated = true;
            FoV = glm::min(FoV + cameraSpeed * deltaTime * 4, 120.0f);
            // camera->MoveForward(cameraSpeed * deltaTime);
        }
        if (!speedRightActivated) {
            speedRightMomentum -= deltaTime;
        }
        speedRightMomentum = glm::min(glm::max(speedRightMomentum, 0.0f), 1.0f);
        speedRight = glm::mix(0.0f, 10.0f, speedRightMomentum) * cameraSpeed;
        camera->TranslateRight(speedRight * deltaTime);

        speedUpActivated = false;
        if (window->KeyHold(GLFW_KEY_Q)) {
            // TODO(student): Translate the camera forward
            speedUpMomentum += deltaTime;
            speedUpActivated = true;
            FoV = glm::min(FoV + cameraSpeed * deltaTime * 4, 120.0f);
            // camera->MoveForward(cameraSpeed * deltaTime);
        }
        if (!speedUpActivated) {
            speedUpMomentum -= deltaTime;
        }
        speedUpMomentum = glm::min(glm::max(speedUpMomentum, 0.0f), 1.0f);
        speedUp = glm::mix(0.0f, 10.0f, speedUpMomentum) * cameraSpeed;
        camera->TranslateUpward(-speedUp * deltaTime);

        speedDownActivated = false;
        if (window->KeyHold(GLFW_KEY_E)) {
            // TODO(student): Translate the camera forward
            speedDownMomentum += deltaTime;
            speedDownActivated = true;
            FoV = glm::min(FoV + cameraSpeed * deltaTime * 4, 120.0f);
            // camera->MoveForward(cameraSpeed * deltaTime);
        }
        if (!speedDownActivated) {
            speedDownMomentum -= deltaTime;
        }
        speedDownMomentum = glm::min(glm::max(speedDownMomentum, 0.0f), 1.0f);
        speedDown = glm::mix(0.0f, 10.0f, speedDownMomentum) * cameraSpeed;
        camera->TranslateUpward(speedDown * deltaTime);

        if (FoV > 60) {
            FoV -= cameraSpeed * deltaTime * 7;
        }

        /*std::cout << "FrontMomentum: " << speedFrontMomentum << std::endl;
        std::cout << "BackMomentum: " << speedBackMomentum << std::endl;
        std::cout << "LeftMomentum: " << speedLeftMomentum << std::endl;
        std::cout << "RightMomentum: " << speedRightMomentum << std::endl;
        std::cout << "UpMomentum: " << speedUpMomentum << std::endl;
        std::cout << "BackMomentum: " << speedBackMomentum << std::endl;*/

    // TODO(student): Change projection parameters. Declare any extra
    // variables you might need in the class header. Inspect this file
    // for any hardcoded projection arguments (can you find any?) and
    // replace them with those extra variables.

}


void DroneGame::OnKeyPress(int key, int mods)
{
    // Add key press event

}


void DroneGame::OnKeyRelease(int key, int mods)
{

}


void DroneGame::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    float sensivityOX = 0.001f;
    float sensivityOY = 0.001f;
    // Add mouse move event

    if (window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT))
    {

        if (window->GetSpecialKeyState() == 0) {
            renderCameraTarget = false;
            // TODO(student): Rotate the camera in first-person mode around
            // OX and OY using `deltaX` and `deltaY`. Use the sensitivity
            // variables for setting up the rotation speed.
            camera->RotateFirstPerson_OY(-deltaX * sensivityOX);
            // camera->RotateFirstPerson_OX(-deltaY * sensivityOX);

        }

        if (window->GetSpecialKeyState() & GLFW_MOD_CONTROL) {
            renderCameraTarget = true;
            // TODO(student): Rotate the camera in third-person mode around
            // OX and OY using `deltaX` and `deltaY`. Use the sensitivity
            // variables for setting up the rotation speed.
            camera->RotateThirdPerson_OY(-deltaX * sensivityOX);

        }
    }
}


void DroneGame::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button press event
}


void DroneGame::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button release event
}


void DroneGame::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
}


void DroneGame::OnWindowResize(int width, int height)
{
}
