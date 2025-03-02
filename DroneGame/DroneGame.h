#pragma once

#include "components/simple_scene.h"
#include "lab_m1/lab5/lab_camera.h"



class DroneGame : public gfxc::SimpleScene
{
public:
    DroneGame();
    ~DroneGame();

    void Init() override;

private:
    void FrameStart() override;
    void Update(float deltaTimeSeconds) override;
    void FrameEnd() override;

    void RenderMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, implemented::Camera* cam, glm::vec3 color);
    void RenderSimpleMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, unsigned int texture1);

    void OnInputUpdate(float deltaTime, int mods) override;
    void OnKeyPress(int key, int mods) override;
    void OnKeyRelease(int key, int mods) override;
    void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
    void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
    void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
    void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
    void OnWindowResize(int width, int height) override;

protected:
    implemented::Camera* camera;
    implemented::Camera* cameraDown;
    implemented::Camera* cameraFrameBuffer;
    glm::mat4 projectionMatrix;
    bool renderCameraTarget;
    glm::vec3 val = {};
    unsigned int framebuffer;
    unsigned int textureColorbuffer;
    unsigned int rbo;
    Mesh* frameBufferMesh;

    float bullet = 0;
    glm::vec3 bomb = glm::vec3(0, -2, 0);
    glm::vec3 enemy;
    glm::vec3 reload;

    // TODO(student): If you need any other class variables, define them here.
    float x = 0;
    float time = 0;
    float arrow_rot = 0;
    Mesh* plane;
    std::vector<glm::vec3> obstacles;
    std::vector<float> scaling;
    float rotation = 0;
    float FoV = 70;
    float dt;
    float speedFrontMomentum = 0;
    float speedFront = 0;
    bool speedFrontActivated = false;
    int collisionDirection = 0;

    float speedBackMomentum = 0;
    float speedBack = 0;
    bool speedBackActivated = false;

    float speedLeftMomentum = 0;
    float speedLeft = 0;
    bool speedLeftActivated = false;

    float speedRightMomentum = 0;
    float speedRight = 0;
    bool speedRightActivated = false;

    float speedUpMomentum = 0;
    float speedUp = 0;
    bool speedUpActivated = false;

    float speedDownMomentum = 0;
    float speedDown = 0;
    bool speedDownActivated = false;
};
