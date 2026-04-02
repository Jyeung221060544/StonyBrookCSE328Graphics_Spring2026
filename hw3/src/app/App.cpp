#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <stdexcept>

#include "app/App.h"
#include "shape/Line.h"
#include "shape/Mesh.h"
#include "shape/Sphere.h"
#include "shape/Tetrahedron.h"
#include "util/Shader.h"
#include "shape/Cylinder.h"
#include "shape/Cone.h"
#include "shape/Torus.h"
#include "shape/Superquadric.h"

static std::vector<Mesh::Vertex> loadPolyhedronMesh(
        const std::string & vertexFile,
        const glm::vec3 & color
)
{
    std::vector<Mesh::Vertex> vertices;

    std::ifstream fin(vertexFile);
    if (!fin)
    {
        throw std::runtime_error("failed to open " + vertexFile);
    }

    glm::vec3 v1, v2, v3;
    while (fin >> v1.x >> v1.y >> v1.z
               >> v2.x >> v2.y >> v2.z
               >> v3.x >> v3.y >> v3.z)
    {
        glm::vec3 fn = glm::normalize(glm::cross(v2 - v1, v3 - v1));

        vertices.emplace_back(v1, fn, color);
        vertices.emplace_back(v2, fn, color);
        vertices.emplace_back(v3, fn, color);
    }

    return vertices;
}

static std::vector<Mesh::Vertex> subdivideUnitSphereMesh(
        const std::vector<Mesh::Vertex> & input,
        const glm::vec3 & color
)
{
    std::vector<Mesh::Vertex> output;
    output.reserve(input.size() * 4);

    for (size_t i = 0; i + 2 < input.size(); i += 3)
    {
        glm::vec3 v1 = glm::normalize(input[i].position);
        glm::vec3 v2 = glm::normalize(input[i + 1].position);
        glm::vec3 v3 = glm::normalize(input[i + 2].position);

        glm::vec3 v12 = glm::normalize((v1 + v2) * 0.5f);
        glm::vec3 v23 = glm::normalize((v2 + v3) * 0.5f);
        glm::vec3 v31 = glm::normalize((v3 + v1) * 0.5f);

        auto addTriangle = [&](const glm::vec3 & a, const glm::vec3 & b, const glm::vec3 & c)
        {
            glm::vec3 fn = glm::normalize(glm::cross(b - a, c - a));
            output.emplace_back(a, fn, color);
            output.emplace_back(b, fn, color);
            output.emplace_back(c, fn, color);
        };

        addTriangle(v1,  v12, v31);
        addTriangle(v2,  v23, v12);
        addTriangle(v3,  v31, v23);
        addTriangle(v12, v23, v31);
    }

    return output;
}

static std::vector<Mesh::Vertex> makeSmoothUnitSphereMesh(
        const std::vector<Mesh::Vertex> & input,
        const glm::vec3 & color
)
{
    std::vector<Mesh::Vertex> output;
    output.reserve(input.size());

    for (const auto & v : input)
    {
        glm::vec3 p = glm::normalize(v.position);
        glm::vec3 n = glm::normalize(p);
        output.emplace_back(p, n, color);
    }

    return output;
}

static std::vector<Mesh::Vertex> makeDisk(
        float radius,
        float y,
        int segments,
        const glm::vec3 & color
)
{
    std::vector<Mesh::Vertex> verts;

    for (int i = 0; i < segments; ++i)
    {
        float a = 2 * M_PI * i / segments;
        float b = 2 * M_PI * (i + 1) / segments;

        glm::vec3 center(0, y, 0);
        glm::vec3 p1(radius * cos(a), y, radius * sin(a));
        glm::vec3 p2(radius * cos(b), y, radius * sin(b));

        glm::vec3 normal = (y > 0) ? glm::vec3(0,1,0) : glm::vec3(0,-1,0);

        verts.emplace_back(center, normal, color);
        verts.emplace_back(p1, normal, color);
        verts.emplace_back(p2, normal, color);
    }

    return verts;
}

static void loadSuperquadricParams(
        float &a, float &b, float &e1, float &e2)
{
    std::ifstream fin("etc/config.txt");
    if (!fin)
        throw std::runtime_error("failed to open config.txt");

    fin >> a >> b >> e1 >> e2;
}

static std::vector<Mesh::Vertex> subdivideTriangleMesh(
        const std::vector<Mesh::Vertex> & input,
        const glm::vec3 & color
)
{
    std::vector<Mesh::Vertex> output;
    output.reserve(input.size() * 4);

    for (size_t i = 0; i + 2 < input.size(); i += 3)
    {
        glm::vec3 v1 = input[i].position;
        glm::vec3 v2 = input[i + 1].position;
        glm::vec3 v3 = input[i + 2].position;

        glm::vec3 v12 = 0.5f * (v1 + v2);
        glm::vec3 v23 = 0.5f * (v2 + v3);
        glm::vec3 v31 = 0.5f * (v3 + v1);

        auto addTriangle = [&](const glm::vec3 & a, const glm::vec3 & b, const glm::vec3 & c)
        {
            glm::vec3 fn = glm::normalize(glm::cross(b - a, c - a));
            output.emplace_back(a, fn, color);
            output.emplace_back(b, fn, color);
            output.emplace_back(c, fn, color);
        };

        addTriangle(v1,  v12, v31);
        addTriangle(v2,  v23, v12);
        addTriangle(v3,  v31, v23);
        addTriangle(v12, v23, v31);
    }

    return output;
}

App & App::getInstance()
{
    static App instance;
    return instance;
}


void App::run()
{
    while (!glfwWindowShouldClose(pWindow))
    {
        // Per-frame logic
        perFrameTimeLogic(pWindow);
        processKeyInput(pWindow);

        // Send render commands to OpenGL server
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        render();

        // Check and call events and swap the buffers
        glfwSwapBuffers(pWindow);
        glfwPollEvents();
    }
}


void App::cursorPosCallback(GLFWwindow * window, double xpos, double ypos)
{
    App & app = *reinterpret_cast<App *>(glfwGetWindowUserPointer(window));

    app.mousePos.x = xpos;
    app.mousePos.y = App::kWindowHeight - ypos;

    if (app.mousePressed)
    {
        // Note: Must calculate offset first, then update lastMouseLeftPressPos.
        glm::dvec2 offset = app.mousePos - app.lastMouseLeftPressPos;
        app.lastMouseLeftPressPos = app.mousePos;
        app.camera.processMouseMovement(offset.x, offset.y);
    }
}


void App::framebufferSizeCallback(GLFWwindow * window, int width, int height)
{
    glViewport(0, 0, width, height);
}


void App::keyCallback(GLFWwindow * window, int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS)
    {
        return;
    }

    App & app = *reinterpret_cast<App *>(glfwGetWindowUserPointer(window));

    switch (key)
    {
        case GLFW_KEY_1:
            app.currentScene = App::SceneMode::P1;
            app.subdivisionLevel = 0;
            app.rebuildScene();
            break;

        case GLFW_KEY_2:
            app.currentScene = App::SceneMode::P2;
            app.subdivisionLevel = 0;
            app.rebuildScene();
            break;

        case GLFW_KEY_3:
            app.currentScene = App::SceneMode::P3;
            app.subdivisionLevel = 0;
            app.rebuildScene();
            break;

        case GLFW_KEY_4:
            app.currentScene = App::SceneMode::P4;
            app.subdivisionLevel = 0;
            app.rebuildScene();
            break;

        case GLFW_KEY_5:
            app.currentScene = App::SceneMode::P5;
            app.subdivisionLevel = 0;
            app.rebuildScene();
            break;

        case GLFW_KEY_6:
            app.currentScene = App::SceneMode::P6;
            app.subdivisionLevel = 0;
            app.rebuildScene();
            break;

        case GLFW_KEY_7:
            app.currentScene = App::SceneMode::P7;
            app.subdivisionLevel = 0;
            app.rebuildScene();
            break;

        case GLFW_KEY_F1:
            app.currentDisplayMode = App::DisplayMode::Wireframe;
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            app.rebuildScene();
            break;

        case GLFW_KEY_F2:
            app.currentDisplayMode = App::DisplayMode::Flat;
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            app.rebuildScene();
            break;

        case GLFW_KEY_F4:
            app.currentDisplayMode = App::DisplayMode::Smooth;
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            app.rebuildScene();
            break;

        case GLFW_KEY_X:
            app.showAxes = !app.showAxes;
            break;

        case GLFW_KEY_EQUAL:
            if ((mods & GLFW_MOD_SHIFT) &&
                (app.currentScene == App::SceneMode::P2 ||
                app.currentScene == App::SceneMode::P3 ||
                app.currentScene == App::SceneMode::P5 ||
                app.currentScene == App::SceneMode::P6))
            {
                if (app.subdivisionLevel < 2)
                {
                    app.subdivisionLevel++;
                    app.rebuildScene();
                }
            }
            break;

        case GLFW_KEY_H:
            if (app.currentScene == App::SceneMode::P7 && !app.flightActive)
            {
                app.flightActive = true;
                app.horizontalLoop = true;
                app.flightAngle = 0.0f;

                app.savedCameraPosition = app.camera.position;
                app.savedCameraFront = app.camera.front;
                app.savedCameraUp = app.camera.up;
            }
            break;

        case GLFW_KEY_V:
            if (app.currentScene == App::SceneMode::P7 && !app.flightActive)
            {
                app.flightActive = true;
                app.horizontalLoop = false;
                app.flightAngle = 0.0f;

                app.savedCameraPosition = app.camera.position;
                app.savedCameraFront = app.camera.front;
                app.savedCameraUp = app.camera.up;
            }
            break;

        default:
            break;
    }
}


void App::mouseButtonCallback(GLFWwindow * window, int button, int action, int mods)
{
    App & app = *reinterpret_cast<App *>(glfwGetWindowUserPointer(window));

    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
        {
            app.mousePressed = true;
            app.lastMouseLeftClickPos = app.mousePos;
            app.lastMouseLeftPressPos = app.mousePos;
        }
        else if (action == GLFW_RELEASE)
        {
            app.mousePressed = false;
        }
    }
}


void App::scrollCallback(GLFWwindow * window, double xoffset, double yoffset)
{
    App & app = *reinterpret_cast<App *>(glfwGetWindowUserPointer(window));
    app.camera.processMouseScroll(yoffset);
}


void App::perFrameTimeLogic(GLFWwindow * window)
{
    App & app = *reinterpret_cast<App *>(glfwGetWindowUserPointer(window));

    double currentFrame = glfwGetTime();
    app.timeElapsedSinceLastFrame = currentFrame - app.lastFrameTimeStamp;
    app.lastFrameTimeStamp = currentFrame;
}


void App::processKeyInput(GLFWwindow * window)
{
    // Camera control
    App & app = *reinterpret_cast<App *>(glfwGetWindowUserPointer(window));

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        app.camera.processKeyboard(Camera::kLeft, app.timeElapsedSinceLastFrame);
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        app.camera.processKeyboard(Camera::kRight, app.timeElapsedSinceLastFrame);
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        app.camera.processKeyboard(Camera::kBackWard, app.timeElapsedSinceLastFrame);
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        app.camera.processKeyboard(Camera::kForward, app.timeElapsedSinceLastFrame);
    }

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        app.camera.processKeyboard(Camera::kUp, app.timeElapsedSinceLastFrame);
    }

    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        app.camera.processKeyboard(Camera::kDown, app.timeElapsedSinceLastFrame);
    }
}


App::App() : Window(kWindowWidth, kWindowHeight, kWindowName, nullptr, nullptr)
{
    // GLFW boilerplate.
    glfwSetWindowUserPointer(pWindow, this);
    glfwSetCursorPosCallback(pWindow, cursorPosCallback);
    glfwSetFramebufferSizeCallback(pWindow, framebufferSizeCallback);
    glfwSetKeyCallback(pWindow, keyCallback);
    glfwSetMouseButtonCallback(pWindow, mouseButtonCallback);
    glfwSetScrollCallback(pWindow, scrollCallback);

    // Global OpenGL pipeline settings
    glViewport(0, 0, kWindowWidth, kWindowHeight);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glLineWidth(2.0f);
    glPointSize(1.0f);
    glEnable(GL_DEPTH_TEST);

    initializeShadersAndObjects();
}


void App::initializeShadersAndObjects()
{
    pLineShader = std::make_unique<Shader>("src/shader/line.vert.glsl",
                                           "src/shader/line.frag.glsl");

    pMeshShader = std::make_unique<Shader>("src/shader/mesh.vert.glsl",
                                           "src/shader/phong.frag.glsl");

    pSphereShader = std::make_unique<Shader>("src/shader/sphere.vert.glsl",
                                             "src/shader/sphere.tesc.glsl",
                                             "src/shader/sphere.tese.glsl",
                                             "src/shader/phong.frag.glsl");
    
    pCylinderShader = std::make_unique<Shader>("src/shader/sphere.vert.glsl",
                                           "src/shader/sphere.tesc.glsl",
                                           "src/shader/cylinder.tese.glsl",
                                           "src/shader/phong.frag.glsl");
    
    pConeShader = std::make_unique<Shader>("src/shader/sphere.vert.glsl",
                                       "src/shader/sphere.tesc.glsl",
                                       "src/shader/cone.tese.glsl",
                                       "src/shader/phong.frag.glsl");

    pTorusShader = std::make_unique<Shader>("src/shader/sphere.vert.glsl",
                                        "src/shader/torus.tesc.glsl",
                                        "src/shader/torus.tese.glsl",
                                        "src/shader/phong.frag.glsl");

    pSuperquadricShader = std::make_unique<Shader>("src/shader/sphere.vert.glsl",
                                        "src/shader/sphere.tesc.glsl",
                                        "src/shader/superquadric.tese.glsl",
                                        "src/shader/phong.frag.glsl"
                                );
    rebuildScene();
    
}

void App::rebuildScene()
{
    shapes.clear();

    // Axes always stored first
    shapes.emplace_back(
            std::make_unique<Line>(
                    pLineShader.get(),
                    std::vector<Line::Vertex> {
                            {{0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
                            {{3.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
                            {{0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
                            {{0.0f, 3.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
                            {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
                            {{0.0f, 0.0f, 3.0f}, {0.0f, 0.0f, 1.0f}},
                    },
                    glm::mat4(1.0f)
            )
    );

    if (currentScene == App::SceneMode::P1)
    {
        shapes.emplace_back(
                std::make_unique<Tetrahedron>(
                        pMeshShader.get(),
                        "var/tetrahedron.txt",
                        glm::translate(glm::mat4(1.0f), {-2.0f, 0.0f, 0.0f})
                )
        );

        shapes.emplace_back(
                std::make_unique<Mesh>(
                        pMeshShader.get(),
                        loadPolyhedronMesh("var/cube.txt", {1.0f, 0.4f, 0.4f}),
                        glm::translate(glm::mat4(1.0f), {0.0f, 0.0f, 0.0f})
                )
        );

        shapes.emplace_back(
                std::make_unique<Mesh>(
                        pMeshShader.get(),
                        loadPolyhedronMesh("var/octahedron.txt", {0.4f, 1.0f, 0.4f}),
                        glm::translate(glm::mat4(1.0f), {2.5f, 0.0f, 0.0f})
                )
        );
    } 
    else if (currentScene == App::SceneMode::P2)
    {
        glm::vec3 color {1.0f, 0.8f, 0.2f};
        auto verts = loadPolyhedronMesh("var/icosahedron.txt", color);

        for (int i = 0; i < subdivisionLevel; ++i)
        {
            verts = subdivideUnitSphereMesh(verts, color);
        }

        if (currentDisplayMode == App::DisplayMode::Smooth)
        {
            verts = makeSmoothUnitSphereMesh(verts, color);
        }
        shapes.emplace_back(
                std::make_unique<Mesh>(
                        pMeshShader.get(),
                        verts,
                        glm::mat4(1.0f)
                )
        );
    }
    else if (currentScene == App::SceneMode::P3)
    {
        glm::vec3 color {0.6f, 0.7f, 1.0f};
        auto verts = loadPolyhedronMesh("var/icosahedron.txt", color);

        for (int i = 0; i < subdivisionLevel; ++i)
        {
            verts = subdivideUnitSphereMesh(verts, color);
        }

        if (currentDisplayMode == App::DisplayMode::Smooth)
        {
            verts = makeSmoothUnitSphereMesh(verts, color);
        }

        glm::mat4 ellipsoidModel = glm::scale(
                glm::mat4(1.0f),
                glm::vec3(1.5f, 1.0f, 0.75f)
        );

        shapes.emplace_back(
                std::make_unique<Mesh>(
                        pMeshShader.get(),
                        verts,
                        ellipsoidModel
                )
        );
    }
    else if (currentScene == App::SceneMode::P4)
    {
        shapes.emplace_back(
                std::make_unique<Sphere>(
                        pSphereShader.get(),
                        glm::vec3(0.0f, 0.0f, 0.0f),
                        1.0f,
                        glm::vec3(1.0f, 0.7f, 0.2f),
                        glm::translate(glm::mat4(1.0f), glm::vec3(-3.0f, 0.0f, 0.0f))
                )
        );

        shapes.emplace_back(
                std::make_unique<Cylinder>(
                        pCylinderShader.get(),
                        glm::vec3(0.0f, 0.0f, 0.0f),
                        1.0f,
                        2.0f,
                        glm::vec3(0.8f, 0.4f, 0.4f),
                        glm::mat4(1.0f)
                )
        );

        // top cap
        shapes.emplace_back(
            std::make_unique<Mesh>(
                pMeshShader.get(),
                makeDisk(1.0f, 1.0f, 40, {0.8f, 0.4f, 0.4f}),
                glm::mat4(1.0f)
            )
        );

        // bottom cap
        shapes.emplace_back(
            std::make_unique<Mesh>(
                pMeshShader.get(),
                makeDisk(1.0f, -1.0f, 40, {0.8f, 0.4f, 0.4f}),
                glm::mat4(1.0f)
            )
        );

        shapes.emplace_back(
            std::make_unique<Cone>(
                    pConeShader.get(),
                    glm::vec3(0.0f, 0.0f, 0.0f),
                    1.0f,
                    2.0f,
                    glm::vec3(0.4f, 0.6f, 1.0f),
                    glm::translate(glm::mat4(1.0f), glm::vec3(3.0f, 0.0f, 0.0f))
            )
        );

        shapes.emplace_back(
                std::make_unique<Mesh>(
                        pMeshShader.get(),
                        makeDisk(1.0f, -1.0f, 40, {0.4f, 0.6f, 1.0f}),
                        glm::translate(glm::mat4(1.0f), glm::vec3(3.0f, 0.0f, 0.0f))
                )
        );
    }
    else if (currentScene == App::SceneMode::P5)
    {
        int tessLevel = 15;
        if (subdivisionLevel == 1) tessLevel = 30;
        if (subdivisionLevel == 2) tessLevel = 60;

        shapes.emplace_back(
                std::make_unique<Torus>(
                        pTorusShader.get(),
                        glm::vec3(0.0f, 0.0f, 0.0f),
                        1.5f,
                        0.5f,
                        tessLevel,
                        glm::vec3(0.8f, 0.3f, 0.8f),
                        glm::mat4(1.0f)
                )
        );
    }
    else if (currentScene == App::SceneMode::P6)
    {
        // Superquadric
        float a, b, e1, e2;
        loadSuperquadricParams(a, b, e1, e2);

        shapes.emplace_back(
                std::make_unique<Superquadric>(
                        pSuperquadricShader.get(),
                        glm::vec3(0.0f, 0.0f, 0.0f),
                        a,
                        b,
                        e1,
                        e2,
                        glm::vec3(0.3f, 0.8f, 0.9f),
                        glm::translate(glm::mat4(1.0f), {-2.5f, 0.0f, 0.0f})
                )
        );

        // Dodecahedron
        glm::vec3 dodeColor {1.0f, 0.8f, 0.3f};
        auto dodeVerts = loadPolyhedronMesh("var/dodecahedron.txt", dodeColor);

        for (int i = 0; i < subdivisionLevel; ++i)
        {
            dodeVerts = subdivideTriangleMesh(dodeVerts, dodeColor);
        }

        shapes.emplace_back(
                std::make_unique<Mesh>(
                        pMeshShader.get(),
                        dodeVerts,
                        glm::translate(glm::mat4(1.0f), {2.5f, 0.0f, 0.0f})
                )
        );
    }
    else if (currentScene == App::SceneMode::P7)
    {
        // Ground block / central plaza
        shapes.emplace_back(
                std::make_unique<Mesh>(
                        pMeshShader.get(),
                        loadPolyhedronMesh("var/cube.txt", {0.6f, 0.6f, 0.6f}),
                        glm::scale(
                            glm::translate(glm::mat4(1.0f), {0.0f, -2.0f, 0.0f}),
                            {8.0f, 0.2f, 8.0f}
                        )
                )
        );

        // Building 1
        shapes.emplace_back(
                std::make_unique<Mesh>(
                        pMeshShader.get(),
                        loadPolyhedronMesh("var/cube.txt", {0.8f, 0.4f, 0.4f}),
                        glm::scale(
                            glm::translate(glm::mat4(1.0f), {-3.0f, 0.0f, -2.0f}),
                            {0.8f, 2.5f, 0.8f}
                        )
                )
        );

        // Building 2
        shapes.emplace_back(
                std::make_unique<Mesh>(
                        pMeshShader.get(),
                        loadPolyhedronMesh("var/cube.txt", {0.4f, 0.8f, 0.4f}),
                        glm::scale(
                            glm::translate(glm::mat4(1.0f), {3.0f, 0.0f, -2.0f}),
                            {0.8f, 3.0f, 0.8f}
                        )
                )
        );

        // Building 3
        shapes.emplace_back(
                std::make_unique<Mesh>(
                        pMeshShader.get(),
                        loadPolyhedronMesh("var/cube.txt", {0.4f, 0.4f, 0.8f}),
                        glm::scale(
                            glm::translate(glm::mat4(1.0f), {-2.0f, 0.0f, 3.0f}),
                            {0.8f, 2.0f, 0.8f}
                        )
                )
        );

        // Dome
        shapes.emplace_back(
                std::make_unique<Sphere>(
                        pSphereShader.get(),
                        glm::vec3(0.0f, 0.0f, 0.0f),
                        1.0f,
                        glm::vec3(1.0f, 0.8f, 0.2f),
                        glm::scale(
                            glm::translate(glm::mat4(1.0f), {0.0f, -0.8f, 3.0f}),
                            {1.2f, 0.7f, 1.2f}
                        )
                )
        );

        // Tower
        shapes.emplace_back(
                std::make_unique<Cylinder>(
                        pCylinderShader.get(),
                        glm::vec3(0.0f, 0.0f, 0.0f),
                        0.6f,
                        2.5f,
                        glm::vec3(0.8f, 0.8f, 0.3f),
                        glm::translate(glm::mat4(1.0f), {2.5f, 0.0f, 2.5f})
                )
        );

        shapes.emplace_back(
                std::make_unique<Mesh>(
                        pMeshShader.get(),
                        makeDisk(0.6f, 1.25f, 40, {0.8f, 0.8f, 0.3f}),
                        glm::translate(glm::mat4(1.0f), {2.5f, 0.0f, 2.5f})
                )
        );

        shapes.emplace_back(
                std::make_unique<Mesh>(
                        pMeshShader.get(),
                        makeDisk(0.6f, -1.25f, 40, {0.8f, 0.8f, 0.3f}),
                        glm::translate(glm::mat4(1.0f), {2.5f, 0.0f, 2.5f})
                )
        );

        // Roof cone
        shapes.emplace_back(
                std::make_unique<Cone>(
                        pConeShader.get(),
                        glm::vec3(0.0f, 0.0f, 0.0f),
                        0.8f,
                        1.5f,
                        glm::vec3(0.9f, 0.5f, 0.2f),
                        glm::translate(glm::mat4(1.0f), {-3.0f, 1.8f, -2.0f})
                )
        );

        shapes.emplace_back(
                std::make_unique<Mesh>(
                        pMeshShader.get(),
                        makeDisk(0.8f, -0.75f, 40, {0.9f, 0.5f, 0.2f}),
                        glm::translate(glm::mat4(1.0f), {-3.0f, 1.8f, -2.0f})
                )
        );

        // Stadium ring
        shapes.emplace_back(
                std::make_unique<Torus>(
                        pTorusShader.get(),
                        glm::vec3(0.0f, 0.0f, 0.0f),
                        1.2f,
                        0.35f,
                        30,
                        glm::vec3(0.8f, 0.3f, 0.8f),
                        glm::translate(glm::mat4(1.0f), {0.0f, -0.5f, -3.5f})
                )
        );

        // Superquadric building
        float a, b, e1, e2;
        loadSuperquadricParams(a, b, e1, e2);
        shapes.emplace_back(
                std::make_unique<Superquadric>(
                        pSuperquadricShader.get(),
                        glm::vec3(0.0f, 0.0f, 0.0f),
                        a, b, e1, e2,
                        glm::vec3(0.3f, 0.8f, 0.9f),
                        glm::translate(glm::mat4(1.0f), {-4.0f, 0.0f, 1.5f})
                )
        );
    }
}

void App::render()
{
    auto t = static_cast<float>(timeElapsedSinceLastFrame);

    // Flight simulation
    if (flightActive && currentScene == SceneMode::P7)
    {
        float speed = 1.0f; // adjust if too slow/fast
        flightAngle += speed * static_cast<float>(timeElapsedSinceLastFrame);

        float radius = 8.0f;

        if (horizontalLoop)
        {
            camera.position.x = radius * cos(flightAngle);
            camera.position.z = radius * sin(flightAngle);
            camera.position.y = 2.0f;

            camera.front = glm::normalize(-camera.position);
        }
        else // vertical loop
        {
            camera.position.x = 0.0f;
            camera.position.y = radius * cos(flightAngle);
            camera.position.z = radius * sin(flightAngle);

            camera.front = glm::normalize(-camera.position);
        }

        // stop after full loop (2π)
        if (flightAngle >= glm::two_pi<float>())
        {
            flightActive = false;

            // restore original camera
            camera.position = savedCameraPosition;
            camera.front = savedCameraFront;
            camera.up = savedCameraUp;
        }
    }

    // Update shader uniforms.
    view = camera.getViewMatrix();
    projection = glm::perspective(glm::radians(camera.zoom),
                                  static_cast<GLfloat>(kWindowWidth) / static_cast<GLfloat>(kWindowHeight),
                                  0.01f,
                                  100.0f);

    pLineShader->use();
    pLineShader->setMat4("view", view);
    pLineShader->setMat4("projection", projection);

    pMeshShader->use();
    pMeshShader->setMat4("view", view);
    pMeshShader->setMat4("projection", projection);
    pMeshShader->setVec3("viewPos", camera.position);
    pMeshShader->setVec3("lightPos", lightPos);
    pMeshShader->setVec3("lightColor", lightColor);

    pSphereShader->use();
    pSphereShader->setMat4("view", view);
    pSphereShader->setMat4("projection", projection);
    pSphereShader->setVec3("viewPos", camera.position);
    pSphereShader->setVec3("lightPos", lightPos);
    pSphereShader->setVec3("lightColor", lightColor);

    pCylinderShader->use();
    pCylinderShader->setMat4("view", view);
    pCylinderShader->setMat4("projection", projection);
    pCylinderShader->setVec3("viewPos", camera.position);
    pCylinderShader->setVec3("lightPos", lightPos);
    pCylinderShader->setVec3("lightColor", lightColor);

    pConeShader->use();
    pConeShader->setMat4("view", view);
    pConeShader->setMat4("projection", projection);
    pConeShader->setVec3("viewPos", camera.position);
    pConeShader->setVec3("lightPos", lightPos);
    pConeShader->setVec3("lightColor", lightColor);

    pTorusShader->use();
    pTorusShader->setMat4("view", view);
    pTorusShader->setMat4("projection", projection);
    pTorusShader->setVec3("viewPos", camera.position);
    pTorusShader->setVec3("lightPos", lightPos);
    pTorusShader->setVec3("lightColor", lightColor);

    pSuperquadricShader->use();
    pSuperquadricShader->setMat4("view", view);
    pSuperquadricShader->setMat4("projection", projection);
    pSuperquadricShader->setVec3("viewPos", camera.position);
    pSuperquadricShader->setVec3("lightPos", lightPos);
    pSuperquadricShader->setVec3("lightColor", lightColor);

    // Render.
    for (size_t i = 0; i < shapes.size(); ++i)
    {
        if (!showAxes && i == 0)
            continue;

        shapes[i]->render(t);
    }
}
