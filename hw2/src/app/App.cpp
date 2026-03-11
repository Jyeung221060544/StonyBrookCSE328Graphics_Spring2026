#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cmath>

#include "app/App.h"
#include "shape/Circle.h"
#include "shape/Triangle.h"
#include "util/Shader.h"

struct BallConfig
{
    float radius;
    glm::vec2 velocity;
};

static BallConfig readBallConfig()
{
    std::ifstream fin("etc/config.txt");
    if (!fin)
    {
        throw std::runtime_error("Failed to open etc/config.txt");
    }

    BallConfig cfg {};
    fin >> cfg.radius >> cfg.velocity.x >> cfg.velocity.y;

    if (!fin)
    {
        throw std::runtime_error("Invalid format in etc/config.txt");
    }

    return cfg;
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
        updateBalls();
        resolveBallCollisions();
        updateFaces();
        resolveFaceCollisions();

        // Send render commands to OpenGL server
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

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
        // // Note: Must calculate offset first, then update lastMouseLeftPressPos.
        // glm::dvec2 offset = app.mousePos - app.lastMouseLeftPressPos;
        app.lastMouseLeftPressPos = app.mousePos;
    }
}


void App::framebufferSizeCallback(GLFWwindow * window, int width, int height)
{
    glViewport(0, 0, width, height);
}


void App::keyCallback(GLFWwindow * window, int key, int scancode, int action, int mods)
{
    App & app = *reinterpret_cast<App *>(glfwGetWindowUserPointer(window));

    if (key == GLFW_KEY_A && action == GLFW_RELEASE)
    {
        app.animationEnabled = !app.animationEnabled;
    }

    if (key == GLFW_KEY_1 && action == GLFW_RELEASE)
    {
        app.currentMode = Mode::BALL;

        BallConfig cfg = readBallConfig();
        std::cout << "BALL mode: r=" << cfg.radius
                << " vx=" << cfg.velocity.x
                << " vy=" << cfg.velocity.y << '\n';
    }

    if (key == GLFW_KEY_3 && action == GLFW_RELEASE)
    {
        app.currentMode = Mode::FACE;
        std::cout << "FACE mode\n";
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

            if (app.currentMode == Mode::BALL)
            {
                app.createBallAt(glm::vec2(app.mousePos));
            }
            else if (app.currentMode == Mode::FACE)
            {
                app.createFaceAt(glm::vec2(app.mousePos));
            }
        }
        else if (action == GLFW_RELEASE)
        {
            app.mousePressed = false;

            #ifdef DEBUG_MOUSE_POS
            std::cout << "[ " << app.mousePos.x << ' ' << app.mousePos.y << " ]\n";
            #endif
        }
    }
}


void App::scrollCallback(GLFWwindow * window, double xoffset, double yoffset)
{

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
    glLineWidth(1.0f);
    glPointSize(1.0f);

    // Initialize shaders and objects-to-render;
    pTriangleShader = std::make_unique<Shader>("src/shader/triangle.vert.glsl",
                                               "src/shader/triangle.frag.glsl");
    pCircleShader = std::make_unique<Shader>("src/shader/circle.vert.glsl",
                                             "src/shader/circle.tesc.glsl",
                                             "src/shader/circle.tese.glsl",
                                             "src/shader/circle.frag.glsl");

    // shapes.emplace_back(
    //         std::make_unique<Triangle>(
    //                 pTriangleShader.get(),
    //                 std::vector<Triangle::Vertex> {
    //                         // Vertex coordinate (screen-space coordinate), Vertex color
    //                         {{200.0f, 326.8f}, {1.0f, 0.0f, 0.0f}},
    //                         {{800.0f, 326.8f}, {0.0f, 1.0f, 0.0f}},
    //                         {{500.0f, 846.4f}, {0.0f, 0.0f, 1.0f}},
    //                 }
    //         )
    // );

    // shapes.emplace_back(
    //         std::make_unique<Circle>(
    //                 pCircleShader.get(),
    //                 std::vector<glm::vec3> {
    //                         // Coordinate (x, y) of the center and the radius (screen-space)
    //                         {200.0f, 326.8f, 200.0f},
    //                         {800.0f, 326.8f, 300.0f},
    //                         {500.0f, 846.4f, 400.0f}
    //                 }
    //         )
    // );
}

bool App::canCreateBallAt(const glm::vec2 & position, float radius) const
{
    if (position.x - radius < 0.0f || position.x + radius > kWindowWidth ||
        position.y - radius < 0.0f || position.y + radius > kWindowHeight)
    {
        return false;
    }

    for (const auto & ball : balls)
    {
        glm::vec2 diff = position - ball.position;
        float distanceSquared = glm::dot(diff, diff);
        float minDistance = radius + ball.radius;

        if (distanceSquared < minDistance * minDistance)
        {
            return false;
        }
    }

    return true;
}

void App::createBallAt(const glm::vec2 & position)
{
    BallConfig cfg = readBallConfig();

    if (!canCreateBallAt(position, cfg.radius))
    {
        std::cout << "Rejected ball creation\n";
        return;
    }

    Ball ball;
    ball.position = position;
    ball.velocity = cfg.velocity;
    ball.radius = cfg.radius;

    ball.shape = std::make_unique<Circle>(
        pCircleShader.get(),
        std::vector<glm::vec3>{
            {position.x, position.y, cfg.radius}
        }
    );

    balls.push_back(std::move(ball));
}

void App::updateBalls()
{
    for (auto & ball : balls)
    {
        ball.position += ball.velocity * static_cast<float>(timeElapsedSinceLastFrame);

        if (ball.position.x - ball.radius < 0.0f)
        {
            ball.position.x = ball.radius;
            ball.velocity.x = -ball.velocity.x;
        }
        else if (ball.position.x + ball.radius > kWindowWidth)
        {
            ball.position.x = kWindowWidth - ball.radius;
            ball.velocity.x = -ball.velocity.x;
        }

        if (ball.position.y - ball.radius < 0.0f)
        {
            ball.position.y = ball.radius;
            ball.velocity.y = -ball.velocity.y;
        }
        else if (ball.position.y + ball.radius > kWindowHeight)
        {
            ball.position.y = kWindowHeight - ball.radius;
            ball.velocity.y = -ball.velocity.y;
        }

        ball.shape = std::make_unique<Circle>(
            pCircleShader.get(),
            std::vector<glm::vec3>{
                {ball.position.x, ball.position.y, ball.radius}
            }
        );
    }
}

void App::resolveBallCollisions()
{
    for (size_t i = 0; i < balls.size(); ++i)
    {
        for (size_t j = i + 1; j < balls.size(); ++j)
        {
            Ball & a = balls[i];
            Ball & b = balls[j];

            glm::vec2 delta = b.position - a.position;
            float distanceSquared = glm::dot(delta, delta);
            float minDistance = a.radius + b.radius;

            if (distanceSquared >= minDistance * minDistance)
            {
                continue;
            }

            float distance = std::sqrt(distanceSquared);

            if (distance == 0.0f)
            {
                continue;
            }

            glm::vec2 normal = delta / distance;
            glm::vec2 relativeVelocity = b.velocity - a.velocity;

            float velocityAlongNormal = glm::dot(relativeVelocity, normal);

            if (velocityAlongNormal >= 0.0f)
            {
                continue;
            }

            glm::vec2 impulse = velocityAlongNormal * normal;

            a.velocity += impulse;
            b.velocity -= impulse;

            float overlap = minDistance - distance;
            glm::vec2 correction = 0.5f * overlap * normal;

            a.position -= correction;
            b.position += correction;
        }
    }
}

void App::drawCircle(float x, float y, float r, const glm::vec3 & color)
{
    Circle c(
        pCircleShader.get(),
        std::vector<glm::vec3>{
            {x, y, r}
        }
    );

    pCircleShader->use();
    pCircleShader->setVec3("color", color);

    c.render(0.0f, false);
}

void App::drawMouth(float x1, float y1, float x2, float y2, float x3, float y3,
                    const glm::vec3 & color)
{
    Triangle t(
        pTriangleShader.get(),
        std::vector<Triangle::Vertex>{
            {{x1, y1}, color},
            {{x2, y2}, color},
            {{x3, y3}, color}
        }
    );

    t.render(0.0f, false);
}

void App::createFaceAt(const glm::vec2 & position)
{
    BallConfig cfg = readBallConfig();

    if (!canCreateFaceAt(position, cfg.radius))
    {
        angerFacesOverlapping(position, cfg.radius);
        return;
    }

    Face face;
    face.position = position;
    face.velocity = cfg.velocity;
    face.radius = cfg.radius;
    face.generation = 1;

    faces.push_back(face);
}

void App::renderFace(const Face & face)
{
    float x = face.position.x;
    float y = face.position.y;
    float r = face.radius;
    float a = face.rotationAngle;
    float c = std::cos(a);
    float s = std::sin(a);

    auto rotateOffset = [&](float ox, float oy) -> glm::vec2
    {
        return glm::vec2(
            x + ox * c - oy * s,
            y + ox * s + oy * c
        );
    };

    float angerLevel = face.angerTimeRemaining / 5.0f;
    if (angerLevel > 1.0f)
    {
        angerLevel = 1.0f;
    }
    glm::vec3 headColor(1.0f, 1.0f - angerLevel, 1.0f - angerLevel);
    glm::vec3 eyeColor(1.0f, 1.0f - 0.5f * angerLevel, 1.0f - 0.5f * angerLevel);
    drawCircle(x, y, r, headColor);

    glm::vec2 m1 = rotateOffset(-0.25f * r, -0.5f * r);
    glm::vec2 m2 = rotateOffset( 0.25f * r, -0.5f * r);
    glm::vec2 m3 = rotateOffset( 0.0f,      -0.75f * r);

    
    glm::vec3 mouthColor(1.0f, 1.0f - angerLevel, 1.0f - angerLevel);
    drawMouth(
        m1.x, m1.y,
        m2.x, m2.y,
        m3.x, m3.y,
        mouthColor
    );

    if (face.generation == 1)
    {
        glm::vec2 leftEyePos = rotateOffset(-0.5f * r, 0.0f);
        glm::vec2 rightEyePos = rotateOffset(0.5f * r, 0.0f);

        drawCircle(leftEyePos.x, leftEyePos.y, 0.5f * r, eyeColor);
        drawCircle(rightEyePos.x, rightEyePos.y, 0.5f * r, eyeColor);
    }
    else if (face.generation == 2)
    {
        glm::vec2 leftEyePos = rotateOffset(-0.5f * r, 0.0f);
        drawCircle(leftEyePos.x, leftEyePos.y, 0.5f * r, eyeColor);

        Face rightFace;
        glm::vec2 rightEyePos = rotateOffset(0.5f * r, 0.0f);
        rightFace.position = rightEyePos;
        rightFace.radius = 0.5f * r;
        rightFace.velocity = glm::vec2(0.0f, 0.0f);
        rightFace.generation = face.generation - 1;
        rightFace.rotationAngle = face.rotationAngle;
        rightFace.angularVelocity = face.angularVelocity;

        renderFace(rightFace);
    }
    else
    {
        Face leftFace;
        glm::vec2 leftEyePos = rotateOffset(-0.5f * r, 0.0f);
        leftFace.position = leftEyePos;
        leftFace.radius = 0.5f * r;
        leftFace.velocity = glm::vec2(0.0f, 0.0f);
        leftFace.generation = 1;
        leftFace.rotationAngle = face.rotationAngle;
        leftFace.angularVelocity = face.angularVelocity;

        Face rightFace;
        glm::vec2 rightEyePos = rotateOffset(0.5f * r, 0.0f);
        rightFace.position = rightEyePos;
        rightFace.radius = 0.5f * r;
        rightFace.velocity = glm::vec2(0.0f, 0.0f);
        rightFace.generation = face.generation - 1;
        rightFace.rotationAngle = face.rotationAngle;
        rightFace.angularVelocity = face.angularVelocity;

        renderFace(leftFace);
        renderFace(rightFace);
    }
}

bool App::canCreateFaceAt(const glm::vec2 & position, float radius) const
{
    if (position.x - radius < 0.0f || position.x + radius > kWindowWidth ||
        position.y - radius < 0.0f || position.y + radius > kWindowHeight)
    {
        return false;
    }

    for (const auto & face : faces)
    {
        glm::vec2 diff = position - face.position;
        float distanceSquared = glm::dot(diff, diff);
        float minDistance = radius + face.radius;

        if (distanceSquared < minDistance * minDistance)
        {
            return false;
        }
    }

    return true;
}

void App::updateFaces()
{
    for (auto & face : faces)
    {
        face.position += face.velocity * static_cast<float>(timeElapsedSinceLastFrame);
        face.rotationAngle += face.angularVelocity * static_cast<float>(timeElapsedSinceLastFrame);

        face.angerTimeRemaining -= static_cast<float>(timeElapsedSinceLastFrame);
        if (face.angerTimeRemaining < 0.0f)
        {
            face.angerTimeRemaining = 0.0f;
        }

        if (face.position.x - face.radius < 0.0f)
        {
            face.position.x = face.radius;
            face.velocity.x = -face.velocity.x;
        }
        else if (face.position.x + face.radius > kWindowWidth)
        {
            face.position.x = kWindowWidth - face.radius;
            face.velocity.x = -face.velocity.x;
        }

        if (face.position.y - face.radius < 0.0f)
        {
            face.position.y = face.radius;
            face.velocity.y = -face.velocity.y;
        }
        else if (face.position.y + face.radius > kWindowHeight)
        {
            face.position.y = kWindowHeight - face.radius;
            face.velocity.y = -face.velocity.y;
        }
    }
}

void App::resolveFaceCollisions()
{
    for (size_t i = 0; i < faces.size(); ++i)
    {
        for (size_t j = i + 1; j < faces.size(); ++j)
        {
            Face & a = faces[i];
            Face & b = faces[j];

            glm::vec2 delta = b.position - a.position;
            float distanceSquared = glm::dot(delta, delta);
            float minDistance = a.radius + b.radius;

            if (distanceSquared >= minDistance * minDistance)
            {
                continue;
            }

            float distance = std::sqrt(distanceSquared);
            if (distance == 0.0f)
            {
                continue;
            }

            glm::vec2 normal = delta / distance;
            glm::vec2 relativeVelocity = b.velocity - a.velocity;
            float velocityAlongNormal = glm::dot(relativeVelocity, normal);

            if (velocityAlongNormal >= 0.0f)
            {
                continue;
            }

            a.generation += 1;
            b.generation += 1;

            glm::vec2 impulse = velocityAlongNormal * normal;
            a.velocity += impulse;
            b.velocity -= impulse;

            float overlap = minDistance - distance;
            glm::vec2 correction = 0.5f * overlap * normal;
            a.position -= correction;
            b.position += correction;
        }
    }
}

void App::angerFacesOverlapping(const glm::vec2 & position, float radius)
{
    for (auto & face : faces)
    {
        glm::vec2 diff = position - face.position;
        float distanceSquared = glm::dot(diff, diff);
        float minDistance = radius + face.radius;

        if (distanceSquared < minDistance * minDistance)
        {
            face.angerTimeRemaining = 5.0f;
        }
    }
}

void App::render()
{
    auto t = static_cast<float>(timeElapsedSinceLastFrame);

    // Update all shader uniforms.
    pTriangleShader->use();
    pTriangleShader->setFloat("windowWidth", kWindowWidth);
    pTriangleShader->setFloat("windowHeight", kWindowHeight);

    pCircleShader->use();
    pCircleShader->setFloat("windowWidth", kWindowWidth);
    pCircleShader->setFloat("windowHeight", kWindowHeight);

    // Render all shapes.
    for (auto & s : shapes)
    {
        s->render(t, animationEnabled);
    }

    for (auto & ball : balls)
    {
        ball.shape->render(0.0f, false);
    }

    for (const auto & face : faces)
    {
        renderFace(face);
    }
}
