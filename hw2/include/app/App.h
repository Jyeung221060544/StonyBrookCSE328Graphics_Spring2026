#ifndef APP_H
#define APP_H

#include <memory>
#include <vector>

#include <glm/glm.hpp>

#include "app/Window.h"


class Shader;
class Renderable;
class Circle;

enum class Mode
{
    NONE,
    BALL,
    FACE
};

struct Ball
{
    glm::vec2 position {0.0f, 0.0f};
    glm::vec2 velocity {0.0f, 0.0f};
    float radius {0.0f};
    std::unique_ptr<Circle> shape {nullptr};
};

struct Face
{
    glm::vec2 position {0.0f, 0.0f};
    glm::vec2 velocity {0.0f, 0.0f};
    float radius {0.0f};
    int generation {1};
    float rotationAngle {0.0f};
    float angularVelocity {1.0f};
    float angerTimeRemaining {0.0f};
};

class App : private Window
{
public:
    static App & getInstance();

    void run();

private:
    static void cursorPosCallback(GLFWwindow *, double, double);
    static void framebufferSizeCallback(GLFWwindow *, int, int);
    static void keyCallback(GLFWwindow *, int, int, int, int);
    static void mouseButtonCallback(GLFWwindow *, int, int, int);
    static void scrollCallback(GLFWwindow *, double, double);

    static void perFrameTimeLogic(GLFWwindow *);
    static void processKeyInput(GLFWwindow *);

    // from CMakeLists.txt, compile definition
    static constexpr char kWindowName[] {WINDOW_NAME};
    static constexpr int kWindowWidth {1000};
    static constexpr int kWindowHeight {1000};

private:
    App();

    void render();
    void createBallAt(const glm::vec2 & position);
    bool canCreateBallAt(const glm::vec2 & position, float radius) const;
    void updateBalls();
    void resolveBallCollisions();
    void createFaceAt(const glm::vec2 & position);
    void renderFace(const Face & face);
    bool canCreateFaceAt(const glm::vec2 & position, float radius) const;
    void updateFaces();
    void resolveFaceCollisions();
    void drawCircle(float x, float y, float r, const glm::vec3 & color);
    void drawMouth(float x1, float y1, float x2, float y2, float x3, float y3,
               const glm::vec3 & color);
    void angerFacesOverlapping(const glm::vec2 & position, float radius);

    // Shaders.
    // In principle, a shader could be reused across multiple objects.
    // Thus, these shaders are not designed as members of object classes.
    std::unique_ptr<Shader> pTriangleShader {nullptr};
    std::unique_ptr<Shader> pCircleShader {nullptr};

    // Objects to render.
    std::vector<std::unique_ptr<Renderable>> shapes;

    Mode currentMode {Mode::NONE};
    std::vector<Ball> balls;
    std::vector<Face> faces;

    // Object attributes affected by GUI.
    bool animationEnabled {true};

    // Frontend GUI
    double timeElapsedSinceLastFrame {0.0};
    double lastFrameTimeStamp {0.0};

    bool mousePressed {false};
    glm::dvec2 mousePos {0.0, 0.0};

    // Note lastMouseLeftClickPos is different from lastMouseLeftPressPos.
    // If you press left button (and hold it there) and move the mouse,
    // lastMouseLeftPressPos gets updated to the current mouse position
    // (while lastMouseLeftClickPos, if there is one, remains the original value).
    glm::dvec2 lastMouseLeftClickPos {0.0, 0.0};
    glm::dvec2 lastMouseLeftPressPos {0.0, 0.0};
};


#endif  // APP_H
