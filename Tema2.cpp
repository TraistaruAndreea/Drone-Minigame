#include "lab_m1/tema2/Tema2.h"
#include "lab_m1/lab3/object2d.h"
#include <random>

using namespace std;
using namespace m1;

Tema2::Tema2()
{
}


Tema2::~Tema2()
{
}

float droneRotation = 0.0f;
enum CameraMode { FIRST_PERSON, THIRD_PERSON };
CameraMode cameraMode = FIRST_PERSON;

struct Obstacle {
    glm::vec3 position;
    glm::vec3 size;
    std::string type;
};
struct FinalPosition {
    glm::vec3 position;
    glm::vec3 size;
    std::string type;
};

std::vector<Obstacle> obstacles;
std::vector<FinalPosition> finalPositions;
std::vector<glm::vec3> cloudPositions;

float angularVelocityX = 0.0f;
float angularVelocityY = 0.0f;
float angularVelocityZ = 0.0f;

float rotationX = 0.0f;
float rotationY = 0.0f;
float rotationZ = 0.0f;

const float gravityEffect = 0.5f; // Gravitatie angulara
const float angularDamping = 0.95f; // Amortizare pentru viteza unghiulara
const float maxAngularVelocity = 2.0f;

float propulseForce = 0.0f;
float gravity = -9.81f;
float altitude = 10.0f;
float altitudeVelocity = 0.0f;  // Viteza de schimbare a altitudinii
const float maxAltitude = 100.0f; // Altitudinea maxima

Mesh* GenerateGrid(int m, int n, float width, float height) {
    vector<VertexFormat> vertices;
    vector<unsigned int> indices;

    for (int i = 0; i <= m; i++) {
        for (int j = 0; j <= n; j++) {
            float x = -25.0f + j * width / n;
            float z = -25.0f + i * height / m;
            float color_variation = (float)i / m;
            vertices.emplace_back(
                glm::vec3(x, 0.0f, z),
                glm::vec3(0.2f, 0.6f, 0.2f),
                glm::vec3(0.0f, 1.0f, 0.0f),
                glm::vec2((float)j / n, (float)i / m));


        }
    }

    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            int topLeft = i * (n + 1) + j;
            int topRight = topLeft + 1;
            int bottomLeft = (i + 1) * (n + 1) + j;
            int bottomRight = bottomLeft + 1;

            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);

            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }
    Mesh* grid = new Mesh("grid");
    grid->InitFromData(vertices, indices);
    return grid;
}

float MyRandom(float x, float y) {
    return glm::fract(sin(x * 12.9898f + y * 78.233f) * 43758.5453123f);
}

float noise(float x, float y) {
    int ix = int(x);
    int iy = int(y);

    float fx = x - ix;
    float fy = y - iy;

    float a = MyRandom(ix, iy);
    float b = MyRandom(ix + 1, iy);
    float c = MyRandom(ix, iy + 1);
    float d = MyRandom(ix + 1, iy + 1);

    float u = fx * fx * (3.0f - 2.0f * fx);
    float v = fy * fy * (3.0f - 2.0f * fy);

    return a * (1.0f - u) * (1.0f - v) +
        b * u * (1.0f - v) +
        c * (1.0f - u) * v +
        d * u * v;
}

float getTerrainHeight(float x, float z) {
    return noise(x * 2.0f, z * 2.0f);
}

void Tema2::Init()
{
    renderCameraTarget = false;
    projectionType = true;

    camera = new implemented::Camera1();
    camera->Set(glm::vec3(0, 2, 3.5f), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0));

	MiniMapCamera = new implemented::Camera1();
	MiniMapCamera->Set(glm::vec3(0, 30, 0), glm::vec3(0, 0, 0), glm::vec3(0, 0, 1));
    projectionMatrix = glm::perspective(RADIANS(60), window->props.aspectRatio, Z_NEAR, Z_FAR);

    Mesh* body1 = object2D::CreateParallelepiped("body1", glm::vec3(-0.1f, 0.0f, -1.0f), 0.2f, 0.1f, 2.0f, glm::vec3(0.5f, 0.5f, 0.5f));
    AddMeshToList(body1);
    Mesh* body2 = object2D::CreateParallelepiped("body2", glm::vec3(-1.0f, 0.0f, -0.1f), 2.0f, 0.1f, 0.2f, glm::vec3(0.5f, 0.5f, 0.5f));
    AddMeshToList(body2);

    Mesh* rotorBase = object2D::CreateCube("rotorBase", glm::vec3(-0.15f, 0.0f, -0.15f), 0.3f, glm::vec3(0.0f, 0.0f, 0.0f));
    AddMeshToList(rotorBase);

    Mesh* rotorBlade = object2D::CreateParallelepiped("rotorBlade", glm::vec3(-0.5f, 0.0f, -0.05f), 1.0f, 0.02f, 0.25f, glm::vec3(0.0f, 0.0f, 0.0f));
    AddMeshToList(rotorBlade);

    Shader* droneShader = new Shader("DroneShader");
    droneShader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "Tema2", "shaders", "DroneVShader.glsl"), GL_VERTEX_SHADER);
    droneShader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "Tema2", "shaders", "DroneFShader.glsl"), GL_FRAGMENT_SHADER);
    droneShader->CreateAndLink();
    shaders["DroneShader"] = droneShader;

    Mesh* terrain = GenerateGrid(50, 50, 100, 100);
    meshes["terrain"] = terrain;

    Shader* shader = new Shader("TerrainShader");
    shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "Tema2", "shaders", "VertexShader.glsl"), GL_VERTEX_SHADER);
    shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "Tema2", "shaders", "FragmentShader.glsl"), GL_FRAGMENT_SHADER);
    shader->CreateAndLink();
    shaders["TerrainShader"] = shader;

	Mesh* treeBase = object2D::CreateCylinder("treeBase", glm::vec3(0.0f, 0.0f, 0.0f), 4.0f, 1.0f, glm::vec3(0.5f, 0.5f, 0.5f), true);
	AddMeshToList(treeBase);

	Mesh* treeCone1 = object2D::CreateCone("treeCone1", glm::vec3(0.0f, 3.0f, 0.0f), 4.0f, 3.0f, glm::vec3(0.0f, 1.0f, 0.0f), true);
	AddMeshToList(treeCone1);

    Shader* obstacle = new Shader("ObstacleShader");
    obstacle->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "Tema2", "shaders", "DroneVShader.glsl"), GL_VERTEX_SHADER);
    obstacle->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "Tema2", "shaders", "DroneFShader.glsl"), GL_FRAGMENT_SHADER);
    obstacle->CreateAndLink();
    shaders["ObstacleShader"] = obstacle;

	Mesh* houseBase = object2D::CreateParallelepiped("houseBase", glm::vec3(35.0f, 0.0f, 17.0f), 3.f, 3.0f, 3.0f, glm::vec3(0.5f, 0.5f, 0.5f));
	AddMeshToList(houseBase);

	Mesh* houseRoof = object2D::CreateRoof("houseRoof", glm::vec3(36.5f, 3.0f, 18.5f), 4.0f, 3.5f, 3.0f, glm::vec3(1.0f, 0.0f, 0.0f), true);
	AddMeshToList(houseRoof);

	Mesh* cloudPart = object2D::CreateSphere("cloudPart", glm::vec3(0.0f, 0.0f, 0.0f), 1.0f, glm::vec3(0.9f, 0.9f, 0.9f), 36, 18);
    AddMeshToList(cloudPart);
	GenerateClouds();

	Shader* shadowShader = new Shader("ShadowShader");
	shadowShader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "Tema2", "shaders", "ShadowVShader.glsl"), GL_VERTEX_SHADER);
	shadowShader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "Tema2", "shaders", "ShadowFShader.glsl"), GL_FRAGMENT_SHADER);
	shadowShader->CreateAndLink();
	shaders["ShadowShader"] = shadowShader;

	droneX = 10.0f;
	droneY = 20.0f;
	droneZ = 0.0f;

    prevX = droneX;
	prevY = droneY;
	prevZ = droneZ;

	obstacles.clear();
	finalPositions.clear();
	GenerateRandomObstacles();
}

void Tema2::FrameStart()
{
    // Clears the color buffer (using the previously set color) and depth buffer
    glClearColor(0.6f, 0.8f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::ivec2 resolution = window->GetResolution();
    // Sets the screen area where to draw
    glViewport(0, 0, resolution.x, resolution.y);
}

void Tema2::FrameEnd()
{
}

bool Tema2::CheckCollision(float droneX, float droneY, float droneZ) {
    if (droneY <= getTerrainHeight(droneX, droneZ)) {
        return true;
    }

    for (const auto& obstacle : finalPositions) {
        glm::vec3 obstacleMin = obstacle.position - obstacle.size;
        glm::vec3 obstacleMax = obstacle.position + obstacle.size;

        bool collisionX = droneX >= obstacleMin.x && droneX <= obstacleMax.x;
        bool collisionY = droneY >= obstacleMin.y && droneY <= obstacleMax.y;
        bool collisionZ = droneZ >= obstacleMin.z && droneZ <=obstacleMax.z;

        if (collisionX && collisionY && collisionZ) {
            return true;
        }
    }
    return false;
}

void Tema2::GenerateClouds() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> xDist(-30.0f, 30.0f);
    std::uniform_real_distribution<float> zDist(-30.0f, 30.0f);
    std::uniform_real_distribution<float> yDist(10.0f, 30.0f);

    cloudPositions.clear();

    for (int i = 0; i < 10; ++i) {
        float x = xDist(gen);
        float y = yDist(gen);
        float z = zDist(gen);
        cloudPositions.push_back(glm::vec3(x, y, z));
    }
}

void Tema2::RenderClouds(float time, implemented::Camera1* MyCamera) {
    for (const auto& pos : cloudPositions) {
        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, pos);

        glm::mat4 part1 = glm::translate(modelMatrix, glm::vec3(-1.0f, 0.0f, 0.0f));
        RenderMeshDrone(meshes["cloudPart"], shaders["DroneShader"], part1, glm::vec3(0.9f, 0.9f, 0.9f), MyCamera);

        glm::mat4 part2 = glm::translate(modelMatrix, glm::vec3(1.0f, 0.0f, 0.0f));
        RenderMeshDrone(meshes["cloudPart"], shaders["DroneShader"], part2, glm::vec3(0.9f, 0.9f, 0.9f), MyCamera);

        glm::mat4 part3 = glm::translate(modelMatrix, glm::vec3(0.0f, 0.5f, 0.0f));
        RenderMeshDrone(meshes["cloudPart"], shaders["DroneShader"], part3, glm::vec3(0.9f, 0.9f, 0.9f), MyCamera);
    }
}

void Tema2::AnimateClouds(float deltaTimeSeconds) {
    for (auto& pos : cloudPositions) {
        pos.x += deltaTimeSeconds * 2.0f;
        pos.z += deltaTimeSeconds * 1.0f;
        if (pos.x > 50.0f) pos.x = -50.0f;
        if (pos.z > 50.0f) pos.z = -50.0f;
    }
}

void Tema2::Update(float deltaTimeSeconds)
{
    static float time = 0;
    time += deltaTimeSeconds;

    if (splitScreen) {
		RenderSplitScreen(Engine::GetElapsedTime());
    }
    else {
        if (cameraMode == FIRST_PERSON) {
            camera->Set(
                glm::vec3(droneX, droneY + 0.2f, droneZ),
                glm::vec3(droneX, droneY + 0.2f, droneZ) + glm::vec3(sin(RADIANS(droneRotation)), 0, -cos(RADIANS(droneRotation))),
                glm::vec3(0, 1, 0)
            );
            MiniMapCamera->Set(
                glm::vec3(droneX, droneY + 30.0f, droneZ),
                glm::vec3(droneX, droneY, droneZ),
                glm::vec3(0, 0, 1)
            );
        }
        else if (cameraMode == THIRD_PERSON) {
            glm::vec3 offset = glm::vec3(-4.0f * cos(droneRotation), 2.0f, -4.0f * sin(droneRotation));
            camera->Set(
                glm::vec3(droneX, droneY + 1.5f, droneZ) + offset,
                glm::vec3(droneX, droneY + 0.5f, droneZ),
                glm::vec3(0, 1, 0)
            );
            MiniMapCamera->Set(
                glm::vec3(droneX, droneY + 30.0f, droneZ),
                glm::vec3(droneX, droneY, droneZ),
                glm::vec3(0, 0, 1)
            );
        }
        if (CheckCollision(droneX, droneY, droneZ)) {
            droneX = prevX;
            droneY = prevY;
            droneZ = prevZ;
        }

        altitudeVelocity += gravity * deltaTimeSeconds;
        altitudeVelocity += propulseForce * deltaTimeSeconds;
        altitude += altitudeVelocity * deltaTimeSeconds;

        if (altitude < 0.0f) altitude = 0.0f;
        if (altitude > maxAltitude) altitude = maxAltitude;

        altitudeVelocity *= 0.98f;

        angularVelocityX *= angularDamping;
        angularVelocityY *= angularDamping;
        angularVelocityZ *= angularDamping;

        angularVelocityX = glm::clamp(angularVelocityX, -maxAngularVelocity, maxAngularVelocity);
        angularVelocityY = glm::clamp(angularVelocityY, -maxAngularVelocity, maxAngularVelocity);
        angularVelocityZ = glm::clamp(angularVelocityZ, -maxAngularVelocity, maxAngularVelocity);

        rotationX += angularVelocityX * deltaTimeSeconds;
        rotationY += angularVelocityY * deltaTimeSeconds;
        rotationZ += angularVelocityZ * deltaTimeSeconds;

        rotationX = glm::mod(rotationX, 360.0f);
        rotationY = glm::mod(rotationY, 360.0f);
        rotationZ = glm::mod(rotationZ, 360.0f);

        RenderClouds(time, camera);
        AnimateClouds(deltaTimeSeconds);

        Shader* shader = shaders["TerrainShader"];
        glUseProgram(shader->GetProgramID());
        int loc_time = glGetUniformLocation(shader->GetProgramID(), "time");
        glUniform1f(loc_time, time);

        glm::mat4 modelMatrix = glm::mat4(1);
        RenderSimpleMesh(meshes["terrain"], shader, modelMatrix, glm::vec3(0.0f, 1.0f, 0.0f), camera);
        RenderObstacles(camera);
        RenderDrone(droneX, droneY + altitude, droneZ, Engine::GetElapsedTime(), camera);
        RenderMinimap(deltaTimeSeconds);
    }

}

void Tema2::GenerateRandomObstacles() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(-25.0f, 25.0f);

    for (int i = 0; i < 15; ++i) {
        float x = dist(gen);
        float z = dist(gen);

        if (x != droneX && z != droneZ) {
            obstacles.push_back({ glm::vec3(x, 0.0f, z), glm::vec3(2.0f, 0.5f, 2.0f), "treeBase" });
            obstacles.push_back({ glm::vec3(x, 1.0f, z), glm::vec3(3.0f, 4.0f, 3.0f), "treeCone1" });
            obstacles.push_back({ glm::vec3(x, 3.0f, z), glm::vec3(3.0f, 4.0f, 3.0f), "treeCone2" });
            obstacles.push_back({ glm::vec3(x, 5.0f, z), glm::vec3(3.0f, 4.0f, 3.0f), "treeCone3" });
        }
    }

    for (int i = 0; i < 10; ++i) {
        float x = dist(gen);
        float z = dist(gen);

        if (x != droneX && z != droneZ) {
            obstacles.push_back({ glm::vec3(x, 0.0f, z), glm::vec3(3.0f, 3.0f, 3.0f), "houseBase" });
            obstacles.push_back({ glm::vec3(x, 0.0f, z), glm::vec3(4.0f, 3.5f, 3.0f), "houseRoof" });
        }
    }
}

void Tema2::RenderObstacles(implemented::Camera1* MyCamera) {
    for (const auto& obstacle : obstacles) {
        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, obstacle.position);

        if (obstacle.type == "treeBase") {
            RenderMeshDrone(meshes["treeBase"], shaders["ObstacleShader"], modelMatrix, glm::vec3(0.4f, 0.26f, 0.13f), MyCamera);
            RenderShadow(modelMatrix, meshes["treeBase"], shaders["ShadowShader"], MyCamera);
            finalPositions.push_back({ obstacle.position, obstacle.size, "treeBase" });
        }
        else if (obstacle.type == "treeCone1" || obstacle.type == "treeCone2" || obstacle.type == "treeCone3") {
            RenderMeshDrone(meshes["treeCone1"], shaders["ObstacleShader"], modelMatrix, glm::vec3(0.0f, 1.0f, 0.0f), MyCamera);
            RenderShadow(modelMatrix, meshes["treeCone1"], shaders["ShadowShader"], MyCamera);
            finalPositions.push_back({ obstacle.position + glm::vec3(0.0f, 2.0f, 0.0f), obstacle.size, obstacle.type });
        }
        else if (obstacle.type == "houseBase") {
            RenderMeshDrone(meshes["houseBase"], shaders["ObstacleShader"], modelMatrix, glm::vec3(0.5f, 0.5f, 0.5f), MyCamera);
			modelMatrix = glm::translate(modelMatrix, glm::vec3(-4.0f, 3.0f, -4.0f));
			RenderShadow(modelMatrix, meshes["houseBase"], shaders["ShadowShader"], MyCamera);
            finalPositions.push_back({ obstacle.position + glm::vec3(35.0f, 0.0f, 17.0f), obstacle.size, "houseBase" });
        }
        else if (obstacle.type == "houseRoof") {
            RenderMeshDrone(meshes["houseRoof"], shaders["ObstacleShader"], modelMatrix, glm::vec3(1.0f, 0.0f, 0.0f), MyCamera);
           // RenderShadow(modelMatrix, meshes["houseRoof"], shaders["ShadowShader"], MyCamera);
            finalPositions.push_back({ obstacle.position + glm::vec3(36.5f, 3.0f, 18.5f), obstacle.size, "houseRoof" });
        }
    }
}

void Tema2::RenderSimpleMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, const glm::vec3& color, implemented::Camera1* MyCamera)
{
    if (!mesh || !shader || !shader->program)
        return;

    shader->Use();
    glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(MyCamera->GetViewMatrix()));
    glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
    glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    int loc_time = glGetUniformLocation(shader->GetProgramID(), "time");
    glUniform1f(loc_time, time);

    int resolution_location = glGetUniformLocation(shader->GetProgramID(), "resolution");
    glUniform2f(resolution_location, window->GetResolution().x, window->GetResolution().y);

    int loc_model = glGetUniformLocation(shader->GetProgramID(), "Model");
    glUniformMatrix4fv(loc_model, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    int loc_view = glGetUniformLocation(shader->GetProgramID(), "View");
    glUniformMatrix4fv(loc_view, 1, GL_FALSE, glm::value_ptr(MyCamera->GetViewMatrix()));

    int loc_proj = glGetUniformLocation(shader->GetProgramID(), "Projection");
    glUniformMatrix4fv(loc_proj, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    int loc_light_pos = glGetUniformLocation(shader->GetProgramID(), "light_position");
    glUniform3f(loc_light_pos, 0.0f, 10.0f, 0.0f);

    int loc_eye_pos = glGetUniformLocation(shader->GetProgramID(), "eye_position");
    glUniform3fv(loc_eye_pos, 1, glm::value_ptr(MyCamera->position));

    int loc_obj_color = glGetUniformLocation(shader->GetProgramID(), "object_color");
    glUniform3f(loc_obj_color, 0.2f, 0.6f, 0.2f);

    mesh->Render();
}

void Tema2::RenderMeshDrone(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, glm::vec3 color, implemented::Camera1* MyCamera)
{
    if (!mesh || !shader || !shader->program)
        return;

    shader->Use();
    glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(MyCamera->GetViewMatrix()));
    glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
    glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    int objectColorLocation = glGetUniformLocation(shader->GetProgramID(), "objectColor");
    if (objectColorLocation != -1) {
        glUniform3fv(objectColorLocation, 1, glm::value_ptr(color));
    }
    mesh->Render();
}

void Tema2::RenderDrone(float droneX, float droneY, float droneZ, float time, implemented::Camera1* MyCamera) {
    glm::mat4 droneModelMatrix = glm::mat4(1);
    droneModelMatrix = glm::translate(droneModelMatrix, glm::vec3(droneX, droneY, droneZ));
    droneModelMatrix = glm::rotate(droneModelMatrix, glm::radians(rotationX), glm::vec3(1, 0, 0));
    droneModelMatrix = glm::rotate(droneModelMatrix, glm::radians(rotationY), glm::vec3(0, 1, 0));
    droneModelMatrix = glm::rotate(droneModelMatrix, glm::radians(rotationZ), glm::vec3(0, 0, 1));

    glm::mat4 bodyMatrix = droneModelMatrix;
    bodyMatrix = glm::rotate(bodyMatrix, glm::radians(30.0f), glm::vec3(0, 1, 0));
    RenderMeshDrone(meshes["body1"], shaders["DroneShader"], bodyMatrix, glm::vec3(0.5f, 0.4f, 0.7f), MyCamera);
    RenderShadow(bodyMatrix, meshes["body1"], shaders["ShadowShader"], MyCamera);

    glm::mat4 body2Matrix = droneModelMatrix;
    body2Matrix = glm::rotate(body2Matrix, glm::radians(45.0f), glm::vec3(0, 1, 0));
    RenderMeshDrone(meshes["body2"], shaders["DroneShader"], body2Matrix, glm::vec3(0.5f, 0.4f, 0.7f), MyCamera);
    RenderShadow(body2Matrix, meshes["body2"], shaders["ShadowShader"], MyCamera);

    for (int i = 0; i < 4; ++i) {
        glm::vec3 offset = glm::vec3(
            cos(RADIANS(45.0f + i * 90.0f)) * 1.1f,
            0.0f,
            sin(RADIANS(45.0f + i * 90.0f)) * 1.1f
        );

        glm::mat4 rotorBaseMatrix = droneModelMatrix;
        rotorBaseMatrix = glm::translate(rotorBaseMatrix, offset);
        RenderMeshDrone(meshes["rotorBase"], shaders["DroneShader"], rotorBaseMatrix, glm::vec3(0.0f, 0.0f, 0.0f), MyCamera);
        RenderShadow(rotorBaseMatrix, meshes["rotorBase"], shaders["ShadowShader"], MyCamera);
    }
    for (int i = 0; i < 4; ++i) {
        glm::vec3 offset = glm::vec3(
            cos(RADIANS(45.0f + i * 90.0f)) * 1.1f,
            0.4f,
            sin(RADIANS(45.0f + i * 90.0f)) * 1.1f
        );

        glm::mat4 rotorBladeMatrix = droneModelMatrix;
        rotorBladeMatrix = glm::translate(rotorBladeMatrix, offset);
        rotorBladeMatrix = glm::rotate(rotorBladeMatrix, time * 5.0f, glm::vec3(0, 1, 0));
        RenderMeshDrone(meshes["rotorBlade"], shaders["DroneShader"], rotorBladeMatrix, glm::vec3(0.0f, 0.0f, 0.0f), MyCamera);
        RenderShadow(rotorBladeMatrix, meshes["rotorBlade"], shaders["ShadowShader"], MyCamera);
    }
}

void Tema2::RenderShadow(glm::mat4 modelMatrix, Mesh* mesh, Shader* shadowShader, implemented::Camera1* MyCamera) {
    if (!mesh || !shadowShader)
        return;

    shadowShader->Use();
    modelMatrix = glm::scale(modelMatrix, glm::vec3(1.2f, 0.0f, 1.2f));
    glUniformMatrix4fv(shadowShader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(MyCamera->GetViewMatrix()));
    glUniformMatrix4fv(shadowShader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
    glUniformMatrix4fv(shadowShader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    GLint loc_model = glGetUniformLocation(shadowShader->program, "model");
    glUniformMatrix4fv(loc_model, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    GLint loc_view = glGetUniformLocation(shadowShader->program, "view");
    glUniformMatrix4fv(loc_view, 1, GL_FALSE, glm::value_ptr(MyCamera->GetViewMatrix()));

    GLint loc_projection = glGetUniformLocation(shadowShader->program, "projection");
    glUniformMatrix4fv(loc_projection, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    mesh->Render();
}

void Tema2::RenderMinimap(float deltaTimeSeconds) {
    glm::ivec2 resolution = window->GetResolution();
    int minimapSize = 300;
    int minimapX = resolution.x - minimapSize;
    int minimapY = resolution.y - minimapSize + 100;

    glViewport(minimapX, minimapY, minimapSize, minimapSize);
    glClear(GL_DEPTH_BUFFER_BIT);

    Shader* shader = shaders["TerrainShader"];
    glUseProgram(shader->GetProgramID());

    glm::mat4 modelMatrix = glm::mat4(1);

    RenderSimpleMesh(meshes["terrain"], shader, modelMatrix, glm::vec3(0.0f, 1.0f, 0.0f), MiniMapCamera);
    RenderDrone(droneX, droneY, droneZ, time, MiniMapCamera);
    RenderObstacles(MiniMapCamera);
    RenderClouds(time, MiniMapCamera);
    AnimateClouds(deltaTimeSeconds);
}

void Tema2::RenderSplitScreen(float deltaTime)
{
    glm::ivec2 resolution = window->GetResolution();
    int screenWidth = resolution.x;
    int screenHeight = resolution.y;

    glViewport(0, 0, screenWidth / 2, screenHeight);

    implemented::Camera1* cameraLeft = new implemented::Camera1();
    cameraLeft->Set(
        glm::vec3(droneX, droneY + 0.2f, droneZ),
        glm::vec3(droneX, droneY + 0.2f, droneZ) + glm::vec3(sin(RADIANS(droneRotation)), 0, -cos(RADIANS(droneRotation))),
        glm::vec3(0, 1, 0)
    );

    if (CheckCollision(droneX, droneY, droneZ)) {
        droneX = prevX;
        droneY = prevY;
        droneZ = prevZ;
    }

    RenderSimpleMesh(meshes["terrain"], shaders["TerrainShader"], glm::mat4(1), glm::vec3(0.0f, 1.0f, 0.0f), cameraLeft);
    RenderDrone(droneX, droneY, droneZ, deltaTime, cameraLeft);
    RenderObstacles(cameraLeft);

    glViewport(screenWidth / 2, 0, screenWidth / 2, screenHeight);
    implemented::Camera1* cameraRight = new implemented::Camera1();
    glm::vec3 offset = glm::vec3(-4.0f * cos(droneRotation), 2.0f, -4.0f * sin(droneRotation));
    cameraRight->Set(
        glm::vec3(droneX, droneY + 1.5f, droneZ) + offset,
        glm::vec3(droneX, droneY + 0.5f, droneZ),
        glm::vec3(0, 1, 0)
    );

    if (CheckCollision(droneX, droneY, droneZ)) {
        droneX = prevX;
        droneY = prevY;
        droneZ = prevZ;
    }

    RenderSimpleMesh(meshes["terrain"], shaders["TerrainShader"], glm::mat4(1), glm::vec3(0.0f, 1.0f, 0.0f), cameraRight);
    RenderDrone(droneX, droneY, droneZ, deltaTime, cameraRight);
    RenderObstacles(cameraRight);
}

void Tema2::OnInputUpdate(float deltaTime, int mods)
{
    float cameraSpeed = 10.0f;
    float rotationSpeed = 2.5f;
    float propulsionForce = 1.0f;
    static float rotationOx = 0.0f;
    static float rotationOz = 0.0f;

    prevX = droneX;
    prevY = droneY;
    prevZ = droneZ;

    if (window->KeyHold(GLFW_KEY_LEFT)) {
        droneRotation += deltaTime * rotationSpeed;
    }
    if (window->KeyHold(GLFW_KEY_RIGHT)) {
        droneRotation -= deltaTime * rotationSpeed;
    }
    if (window->KeyHold(GLFW_KEY_UP)) {
        rotationOx += deltaTime * rotationSpeed; 
    }
    if (window->KeyHold(GLFW_KEY_DOWN)) {
        rotationOx -= deltaTime * rotationSpeed;
    }
    if (window->KeyHold(GLFW_KEY_Z)) {
        rotationOz += deltaTime * rotationSpeed;
    }
    if (window->KeyHold(GLFW_KEY_X)) {
        rotationOz -= deltaTime * rotationSpeed;
    }

    if (window->KeyHold(GLFW_KEY_SPACE)) {
        propulsionForce = 15.0f;
    }
    else {
        propulsionForce = 0.0f;
    }

    float verticalAcceleration = propulsionForce + gravity;
    droneY += verticalAcceleration * deltaTime;

    glm::vec3 forwardDirection(
        sin(droneRotation) * cos(rotationOx),
        sin(rotationOx),
        -cos(droneRotation) * cos(rotationOx)
    );
    glm::vec3 rightDirection(
        cos(droneRotation),
        0,
        sin(droneRotation)
    );
    glm::vec3 upDirection(
        -sin(rotationOz),
        cos(rotationOz),
        0
    );

    if (window->KeyHold(GLFW_KEY_W)) {
        glm::vec3 forwardMove = forwardDirection * cameraSpeed * deltaTime;
        droneX += forwardMove.x;
        droneY += forwardMove.y;
        droneZ += forwardMove.z;
	
    }
    if (window->KeyHold(GLFW_KEY_S)) {
        glm::vec3 backwardMove = -forwardDirection * cameraSpeed * deltaTime;
        droneX += backwardMove.x;
        droneY += backwardMove.y;
        droneZ += backwardMove.z;
    }
    if (window->KeyHold(GLFW_KEY_A)) {
        glm::vec3 leftMove = -rightDirection * cameraSpeed * deltaTime;
        droneX += leftMove.x;
        droneZ += leftMove.z;
    }
    if (window->KeyHold(GLFW_KEY_D)) {
        glm::vec3 rightMove = rightDirection * cameraSpeed * deltaTime;
        droneX += rightMove.x;
        droneZ += rightMove.z;
    }

    if (window->KeyHold(GLFW_KEY_Q)) {
        droneY -= cameraSpeed * deltaTime;
    }
    if (window->KeyHold(GLFW_KEY_E)) {
        droneY += cameraSpeed * deltaTime;
    }

    if (window->KeyHold(GLFW_KEY_N)) {
        splitScreen = true;
    }
    
	if (window->KeyHold(GLFW_KEY_M)) {
		splitScreen = false;
	}
   
}

void Tema2::OnKeyPress(int key, int mods)
{
    if (key == GLFW_KEY_C) {
        if (cameraMode == FIRST_PERSON) {
            cameraMode = THIRD_PERSON;
        }
        else {
            cameraMode = FIRST_PERSON;
        }
    }

}

void Tema2::OnKeyRelease(int key, int mods)
{
}

void Tema2::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    // Add mouse move event
}

void Tema2::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button press event
}

void Tema2::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button release event
}

void Tema2::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
}

void Tema2::OnWindowResize(int width, int height)
{
}