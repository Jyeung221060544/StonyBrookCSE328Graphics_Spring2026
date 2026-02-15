#ifndef APP_H
#define APP_H

#include <memory>
#include <vector>
#include <string>

#include <glm/glm.hpp>

#include "app/Window.h"
#include "shape/Pixel.h"


class Shader;
class Renderable;



class App : private Window
{
public:
    static App & getInstance();

    void run();

private:
    // GLFW callbacks.
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
    /// Bresenham line-drawing algorithm for line (x0, y0) -> (x1, y1) in screen space,
    /// given that its slope m satisfies 0.0 <= m <= 1.0 and that (x0, y0) is the start position.
    /// All pixels on this line are appended to path.
    static void bresenhamLine(std::vector<Pixel::Vertex> & path, int x0, int y0, int x1, int y1);
    static void bresenhamLineWorld(std::vector<Pixel::Vertex>& path,
                               int x0, int y0, int x1, int y1);
    
    static void midpointCircle(std::vector<Pixel::Vertex>& path, int cx, int cy, int r);
    static void plotCircle8(std::vector<Pixel::Vertex>& path, int cx, int cy, int x, int y);

    static void midpointEllipse(std::vector<Pixel::Vertex>& path, int cx, int cy, int a, int b);
    static void plotEllipse4(std::vector<Pixel::Vertex>& path, int cx, int cy, int x, int y);

    static void drawQuadratic(std::vector<Pixel::Vertex>& path, double a2, double a1, double a0);
    static void drawCubic(std::vector<Pixel::Vertex>& path, double a3, double a2, double a1, double a0);
    static void drawSuperquadric(std::vector<Pixel::Vertex>& path,
                             double a, double b, double n);

    static inline bool worldToScreen(int wx, int wy, int& sx, int& sy)
    {
        sx = wx + kWindowWidth / 2;
        sy = wy + kWindowHeight / 2;
        return (0 <= sx && sx < kWindowWidth && 0 <= sy && sy < kWindowHeight);
    }

    // Push a world-space pixel into the path (clipped to viewport).
    static inline void pushWorld(std::vector<Pixel::Vertex>& path, int wx, int wy)
    {
        int sx, sy;
        if (worldToScreen(wx, wy, sx, sy)) {
            path.emplace_back(sx, sy, 1.0f, 1.0f, 1.0f);
        }
    }

    App();

    void render();

    // Shaders.
    // In principle, a shader could be reused across multiple objects.
    // Thus, these shaders are not designed as members of object classes.
    std::unique_ptr<Shader> pPixelShader {nullptr};

    // Objects to render.
    std::vector<std::unique_ptr<Renderable>> shapes;

    // Object attributes affected by GUI.
    bool animationEnabled {true};
    bool showPreview {false};
    int mode = {1}; // 1=line, 3=polyline, 4=circle/ellipse

    // Frontend GUI
    double timeElapsedSinceLastFrame {0.0};
    double lastFrameTimeStamp {0.0};

    bool mousePressed {false};
    glm::dvec2 mousePos {0.0, 0.0};

    std::vector<glm::ivec2> polyPoints;
    bool cHeld {false};

    bool shiftHeld {false};
    bool circleHasCenter {false};
    glm::ivec2 circleCenter {0, 0};

    bool ellipseHasCenter {false};
    glm::ivec2 ellipseCenter {0, 0};


    int curveType {0};                 // 1=cubic, 2=quadratic, 3=superquadric
    std::vector<double> curveParams;   // remaining floats
    bool hasCurveConfig {false};

    static bool loadCurveConfig(const std::string& path, int& typeOut, std::vector<double>& paramsOut);


    // Note lastMouseLeftClickPos is different from lastMouseLeftPressPos.
    // If you press left button (and hold it there) and move the mouse,
    // lastMouseLeftPressPos gets updated to the current mouse position
    // (while lastMouseLeftClickPos, if there is one, remains the original value).
    glm::dvec2 lastMouseLeftClickPos {0.0, 0.0};
    glm::dvec2 lastMouseLeftPressPos {0.0, 0.0};
};


#endif  // APP_H
