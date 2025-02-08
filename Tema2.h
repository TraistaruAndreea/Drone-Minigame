#pragma once

#include "components/simple_scene.h"
#include "lab_m1/tema2/lab_camera.h"
#include "components/transform.h"

#define Z_FAR		(200.f)
#define Z_NEAR		(.01f)

namespace m1
{
    class Tema2 : public gfxc::SimpleScene
    {
    public:
        Tema2();
        ~Tema2();

        void Init() override;

    private:
        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void FrameEnd() override;

        void RenderSimpleMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, const glm::vec3& color, implemented::Camera1 *MyCamera);
        void Tema2::RenderDrone(float droneX, float droneY, float droneZ, float time, implemented::Camera1* MyCamera);
        void Tema2::RenderMeshDrone(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, glm::vec3 color, implemented::Camera1* MyCamera);
        void Tema2::RenderShadow(glm::mat4 modelMatrix, Mesh* mesh, Shader* shadowShader, implemented::Camera1* camera);
        void Tema2::RenderSplitScreen(float deltaTime);
        void Tema2::GenerateRandomObstacles();
        void Tema2::RenderObstacles(implemented::Camera1* MyCamera);
        bool Tema2::CheckCollision(float x, float y, float z);
        void Tema2::GenerateClouds();
        void Tema2::RenderClouds(float time, implemented::Camera1* MyCamera);
        void Tema2::AnimateClouds(float deltaTimeSeconds);
        void Tema2::RenderMinimap(float deltaTimeSeconds);

        void OnInputUpdate(float deltaTime, int mods) override;
        void OnKeyPress(int key, int mods) override;
        void OnKeyRelease(int key, int mods) override;
        void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
        void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
        void OnWindowResize(int width, int height) override;

    protected:
        implemented::Camera1* camera;
        implemented::Camera1* MiniMapCamera;
        glm::mat4 projectionMatrix;
        bool renderCameraTarget;
        bool projectionType;
		float droneX, droneY, droneZ;
        float prevX, prevY, prevZ;
        int time;
        bool splitScreen = false;

        // TODO(student): If you need any other class variables, define them here.
        GLfloat fov;
        GLfloat left, right;
        GLfloat bottom, top;

    };
}   // namespace m1
