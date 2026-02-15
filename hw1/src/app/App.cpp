#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "app/App.h"
#include "shape/Pixel.h"
#include "util/Shader.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>

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

    // Display a preview line which moves with the mouse cursor iff.
    // the most-recent mouse click is left click.
    // showPreview is controlled by mouseButtonCallback.
    if (app.mode == 1 && app.showPreview) 
    {
        auto pixel = dynamic_cast<Pixel *>(app.shapes.front().get());

        auto x0 = static_cast<int>(app.lastMouseLeftPressPos.x);
        auto y0 = static_cast<int>(app.lastMouseLeftPressPos.y);
        auto x1 = static_cast<int>(app.mousePos.x);
        auto y1 = static_cast<int>(app.mousePos.y);

        pixel->path.clear();
        bresenhamLine(pixel->path, x0, y0, x1, y1);
        pixel->dirty = true;
    }

    if (app.mode == 3 && app.showPreview && !app.polyPoints.empty())
    {
        auto pixel = dynamic_cast<Pixel *>(app.shapes.front().get());
        pixel->path.clear();

        // draw existing segments
        for (size_t i = 1; i < app.polyPoints.size(); ++i) {
            app.bresenhamLine(pixel->path,
                app.polyPoints[i-1].x, app.polyPoints[i-1].y,
                app.polyPoints[i].x,   app.polyPoints[i].y);
        }

        // draw preview segment
        auto last = app.polyPoints.back();
        app.bresenhamLine(pixel->path,
            last.x, last.y,
            static_cast<int>(app.mousePos.x),
            static_cast<int>(app.mousePos.y));

        pixel->dirty = true;
    }

    if (app.mode == 4 && app.shiftHeld && app.circleHasCenter)
    {
        auto pixel = dynamic_cast<Pixel *>(app.shapes.front().get());
        pixel->path.clear();

        int cx = app.circleCenter.x;
        int cy = app.circleCenter.y;

        int mx = static_cast<int>(app.mousePos.x);
        int my = static_cast<int>(app.mousePos.y);

        int dx = mx - cx;
        int dy = my - cy;
        int r = static_cast<int>(std::round(std::sqrt(dx*dx + dy*dy)));

        midpointCircle(pixel->path, cx, cy, r);
        pixel->dirty = true;
    }

    if (app.mode == 4 && !app.shiftHeld && app.ellipseHasCenter)
    {
        auto pixel = dynamic_cast<Pixel *>(app.shapes.front().get());
        pixel->path.clear();

        int cx = app.ellipseCenter.x;
        int cy = app.ellipseCenter.y;

        int mx = static_cast<int>(app.mousePos.x);
        int my = static_cast<int>(app.mousePos.y);

        int a = std::abs(mx - cx);
        int b = std::abs(my - cy);

        midpointEllipse(pixel->path, cx, cy, a, b);
        pixel->dirty = true;
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
    if (key == GLFW_KEY_C) {
        if (action == GLFW_PRESS)   app.cHeld = true;
        if (action == GLFW_RELEASE) app.cHeld = false;
    }
    if (key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT) {
        if (action == GLFW_PRESS)   app.shiftHeld = true;
        if (action == GLFW_RELEASE) app.shiftHeld = false;
    }
    if (key == GLFW_KEY_4 && action == GLFW_PRESS) {
        app.circleHasCenter = false;
        app.ellipseHasCenter = false;
    }


    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_1 || key == GLFW_KEY_3 || key == GLFW_KEY_4) {
            app.showPreview = false;
            auto preview = dynamic_cast<Pixel*>(app.shapes.front().get());
            if (preview) { preview->path.clear(); preview->dirty = true; }
        }
        if (key == GLFW_KEY_1) app.mode = 1;
        if (key == GLFW_KEY_3) app.mode = 3;
        if (key == GLFW_KEY_4) app.mode = 4;
        if (key == GLFW_KEY_1 || key == GLFW_KEY_4) {
            app.polyPoints.clear();
            app.cHeld = false;
        }
        if (key == GLFW_KEY_1 || key == GLFW_KEY_3) {
            app.circleHasCenter = false;
            app.shiftHeld = false;
        }
    }



}


void App::mouseButtonCallback(GLFWwindow * window, int button, int action, int mods)
{

    App & app = *reinterpret_cast<App *>(glfwGetWindowUserPointer(window));

    if (app.mode == 1) {
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            if (action == GLFW_PRESS)
            {
                app.mousePressed = true;
                app.lastMouseLeftClickPos = app.mousePos;
                app.lastMouseLeftPressPos = app.mousePos;
            }
            else if (action == GLFW_RELEASE)
            {
                app.mousePressed = false;
                app.showPreview = true;

                #ifdef DEBUG_MOUSE_POS
                std::cout << "[ " << app.mousePos.x << ' ' << app.mousePos.y << " ]\n";
                #endif
            }
        }

        if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
            // finalize current preview into a permanent shape
            app.showPreview = false;

            int x0 = static_cast<int>(app.lastMouseLeftPressPos.x);
            int y0 = static_cast<int>(app.lastMouseLeftPressPos.y);
            int x1 = static_cast<int>(app.mousePos.x);
            int y1 = static_cast<int>(app.mousePos.y);

            auto finalized = std::make_unique<Pixel>(app.pPixelShader.get());
            app.bresenhamLine(finalized->path, x0, y0, x1, y1);
            finalized->dirty = true;

            app.shapes.emplace_back(std::move(finalized));
        }
    } else if (app.mode == 3) {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
            app.polyPoints.emplace_back(
                static_cast<int>(app.mousePos.x),
                static_cast<int>(app.mousePos.y)
            );
            app.showPreview = true;
        }


        if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
            app.polyPoints.emplace_back(
                static_cast<int>(app.mousePos.x),
                static_cast<int>(app.mousePos.y)
            );

            if (app.polyPoints.size() >= 2) {
                auto finalized = std::make_unique<Pixel>(app.pPixelShader.get());

                for (size_t i = 1; i < app.polyPoints.size(); ++i) {
                    app.bresenhamLine(finalized->path,
                        app.polyPoints[i-1].x, app.polyPoints[i-1].y,
                        app.polyPoints[i].x,   app.polyPoints[i].y);
                }

                if (app.cHeld && app.polyPoints.size() >= 3) {
                    // connect last to first
                    app.bresenhamLine(finalized->path,
                        app.polyPoints.back().x, app.polyPoints.back().y,
                        app.polyPoints.front().x, app.polyPoints.front().y);
                }

                finalized->dirty = true;
                app.shapes.emplace_back(std::move(finalized));
            }

            app.polyPoints.clear();
            app.showPreview = false;

            auto preview = dynamic_cast<Pixel*>(app.shapes.front().get());
            if (preview) { preview->path.clear(); preview->dirty = true; }
        }
    } else if (app.mode == 4 && app.shiftHeld) {

        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
        {
            app.ellipseHasCenter = false;
            auto preview = dynamic_cast<Pixel*>(app.shapes.front().get());
            if (preview) { preview->path.clear(); preview->dirty = true; }

            app.circleCenter = glm::ivec2(
                static_cast<int>(app.mousePos.x),
                static_cast<int>(app.mousePos.y)
            );
            app.circleHasCenter = true;
            app.showPreview = true;
        }

        if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
        {
            if (!app.circleHasCenter) return;

            int cx = app.circleCenter.x;
            int cy = app.circleCenter.y;
            int mx = static_cast<int>(app.mousePos.x);
            int my = static_cast<int>(app.mousePos.y);

            int dx = mx - cx;
            int dy = my - cy;
            int r = static_cast<int>(std::round(std::sqrt(dx*dx + dy*dy)));

            auto finalized = std::make_unique<Pixel>(app.pPixelShader.get());
            midpointCircle(finalized->path, cx, cy, r);
            finalized->dirty = true;
            app.shapes.emplace_back(std::move(finalized));

            // reset state + clear preview layer
            app.circleHasCenter = false;
            app.showPreview = false;

            auto preview = dynamic_cast<Pixel*>(app.shapes.front().get());
            if (preview) { preview->path.clear(); preview->dirty = true; }
        }
    } else if (app.mode == 4 && !app.shiftHeld) {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
        {
            app.circleHasCenter = false;
            auto preview = dynamic_cast<Pixel*>(app.shapes.front().get());
            if (preview) { preview->path.clear(); preview->dirty = true; }

            app.ellipseCenter = glm::ivec2(
                static_cast<int>(app.mousePos.x),
                static_cast<int>(app.mousePos.y)
            );
            app.ellipseHasCenter = true;
            app.showPreview = true;
        }

        if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
        {
            if (!app.ellipseHasCenter) return;

            int cx = app.ellipseCenter.x;
            int cy = app.ellipseCenter.y;

            int mx = static_cast<int>(app.mousePos.x);
            int my = static_cast<int>(app.mousePos.y);

            int a = std::abs(mx - cx);
            int b = std::abs(my - cy);

            auto finalized = std::make_unique<Pixel>(app.pPixelShader.get());
            midpointEllipse(finalized->path, cx, cy, a, b);
            finalized->dirty = true;
            app.shapes.emplace_back(std::move(finalized));

            app.ellipseHasCenter = false;
            app.showPreview = false;

            auto preview = dynamic_cast<Pixel*>(app.shapes.front().get());
            if (preview) { preview->path.clear(); preview->dirty = true; }
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

void App::bresenhamLine(std::vector<Pixel::Vertex>& path, int x0, int y0, int x1, int y1)
{
    // Handle vertical/horizontal and all slopes using generalized Bresenham
    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);

    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;

    // If steep, swap x and y, so that it still “walks” along the major axis.
    bool steep = dy > dx;
    if (steep) {
        std::swap(x0, y0);
        std::swap(x1, y1);
        std::swap(dx, dy);
        std::swap(sx, sy);
    }

    int err = 2 * dy - dx;
    int y = y0;
    int x = x0;

    for (int i = 0; i <= dx; ++i) {
        if (steep) path.emplace_back(y, x, 1.0f, 1.0f, 1.0f);
        else       path.emplace_back(x, y, 1.0f, 1.0f, 1.0f);

        if (err > 0) {
            y += sy;
            err -= 2 * dx;
        }
        err += 2 * dy;
        x += sx;
    }
}

void App::plotCircle8(std::vector<Pixel::Vertex>& path, int cx, int cy, int x, int y)
{
    path.emplace_back(cx + x, cy + y, 1.0f, 1.0f, 1.0f);
    path.emplace_back(cx - x, cy + y, 1.0f, 1.0f, 1.0f);
    path.emplace_back(cx + x, cy - y, 1.0f, 1.0f, 1.0f);
    path.emplace_back(cx - x, cy - y, 1.0f, 1.0f, 1.0f);

    path.emplace_back(cx + y, cy + x, 1.0f, 1.0f, 1.0f);
    path.emplace_back(cx - y, cy + x, 1.0f, 1.0f, 1.0f);
    path.emplace_back(cx + y, cy - x, 1.0f, 1.0f, 1.0f);
    path.emplace_back(cx - y, cy - x, 1.0f, 1.0f, 1.0f);
}

void App::midpointCircle(std::vector<Pixel::Vertex>& path, int cx, int cy, int r)
{
    if (r < 0) return;

    int x = 0;
    int y = r;
    int d = 1 - r; // midpoint decision parameter

    plotCircle8(path, cx, cy, x, y);

    while (x < y)
    {
        ++x;
        if (d < 0) {
            d += 2 * x + 1;
        } else {
            --y;
            d += 2 * (x - y) + 1;
        }
        plotCircle8(path, cx, cy, x, y);
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

    // Read bonus curve config
    hasCurveConfig = loadCurveConfig("etc/config.txt", curveType, curveParams);

    std::cout << "[config] loaded=" << hasCurveConfig
              << " type=" << curveType
              << " params=" << curveParams.size() << "\n";
    for (size_t i = 0; i < curveParams.size(); ++i) {
        std::cout << "  p[" << i << "]=" << curveParams[i] << "\n";
    }


    // Global OpenGL pipeline settings
    glViewport(0, 0, kWindowWidth, kWindowHeight);
    glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    glLineWidth(1.0f);
    glPointSize(1.0f);

    // Initialize shaders and objects-to-render;
    pPixelShader = std::make_unique<Shader>("src/shader/pixel.vert.glsl",
                                            "src/shader/pixel.frag.glsl");

    shapes.emplace_back(std::make_unique<Pixel>(pPixelShader.get()));

    if (hasCurveConfig && curveType == 1 && curveParams.size() >= 4) {
        auto curve = std::make_unique<Pixel>(pPixelShader.get());
        drawCubic(curve->path, curveParams[0], curveParams[1], curveParams[2], curveParams[3]);
        curve->dirty = true;
        shapes.emplace_back(std::move(curve));
    }

    if (hasCurveConfig && curveType == 2 && curveParams.size() >= 3) {
        auto curve = std::make_unique<Pixel>(pPixelShader.get());
        drawQuadratic(curve->path, curveParams[0], curveParams[1], curveParams[2]);
        std::cout << "curve points = " << curve->path.size() << "\n";
        curve->dirty = true;
        shapes.emplace_back(std::move(curve));
    }

    if (hasCurveConfig && curveType == 3 && curveParams.size() >= 3) {
        auto curve = std::make_unique<Pixel>(pPixelShader.get());
        drawSuperquadric(curve->path, curveParams[0], curveParams[1], curveParams[2]);
        curve->dirty = true;
        shapes.emplace_back(std::move(curve));
    }


}

void App::plotEllipse4(std::vector<Pixel::Vertex>& path, int cx, int cy, int x, int y)
{
    path.emplace_back(cx + x, cy + y, 1.0f, 1.0f, 1.0f);
    path.emplace_back(cx - x, cy + y, 1.0f, 1.0f, 1.0f);
    path.emplace_back(cx + x, cy - y, 1.0f, 1.0f, 1.0f);
    path.emplace_back(cx - x, cy - y, 1.0f, 1.0f, 1.0f);
}

void App::midpointEllipse(std::vector<Pixel::Vertex>& path, int cx, int cy, int a, int b)
{
    if (a < 0) a = -a;
    if (b < 0) b = -b;

    // Degenerate cases:
    if (a == 0 && b == 0) {
        path.emplace_back(cx, cy, 1.0f, 1.0f, 1.0f);
        return;
    }
    if (a == 0) {
        // vertical line
        for (int y = -b; y <= b; ++y) path.emplace_back(cx, cy + y, 1.0f, 1.0f, 1.0f);
        return;
    }
    if (b == 0) {
        // horizontal line
        for (int x = -a; x <= a; ++x) path.emplace_back(cx + x, cy, 1.0f, 1.0f, 1.0f);
        return;
    }

    long long a2 = 1LL * a * a;
    long long b2 = 1LL * b * b;

    int x = 0;
    int y = b;

    // Region 1 decision parameter
    long long dx = 2LL * b2 * x;
    long long dy = 2LL * a2 * y;
    long long d1 = b2 - a2 * b + a2 / 4; // (b^2) - (a^2 b) + (1/4 a^2)

    plotEllipse4(path, cx, cy, x, y);

    // Region 1: slope magnitude < 1  => dx < dy
    while (dx < dy)
    {
        ++x;
        dx += 2LL * b2;

        if (d1 < 0) {
            d1 += dx + b2;
        } else {
            --y;
            dy -= 2LL * a2;
            d1 += dx - dy + b2;
        }

        plotEllipse4(path, cx, cy, x, y);
    }

    // Region 2 decision parameter
    long long d2 = b2 * (1LL * x * x + x) + b2 / 4
                 + a2 * (1LL * (y - 1) * (y - 1))
                 - a2 * b2;

    // Region 2: slope magnitude >= 1
    while (y > 0)
    {
        --y;
        dy -= 2LL * a2;

        if (d2 > 0) {
            d2 += a2 - dy;
        } else {
            ++x;
            dx += 2LL * b2;
            d2 += dx - dy + a2;
        }

        plotEllipse4(path, cx, cy, x, y);
    }
}

void App::render()
{
    // Update all shader uniforms.
    pPixelShader->use();
    pPixelShader->setFloat("windowWidth", kWindowWidth);
    pPixelShader->setFloat("windowHeight", kWindowHeight);

    // Render all shapes.
    for (auto & s : shapes)
    {
        s->render();
    }
}

// Bonus
bool App::loadCurveConfig(const std::string& path, int& typeOut, std::vector<double>& paramsOut)
{
    std::ifstream fin(path);
    if (!fin.is_open()) return false;

    std::string line;
    if (!std::getline(fin, line)) return false;

    std::istringstream iss(line);

    int t;
    if (!(iss >> t)) return false;

    paramsOut.clear();
    double v;
    while (iss >> v) paramsOut.push_back(v);

    typeOut = t;
    return true;
}

void App::drawQuadratic(std::vector<Pixel::Vertex>& path, double a2, double a1, double a0)
{
    path.clear();

    int xMin = -kWindowWidth / 2;
    int xMax =  kWindowWidth / 2 - 1;

    int prevX = xMin;
    int prevY = static_cast<int>(std::lround(a2 * prevX * prevX + a1 * prevX + a0));

    for (int x = xMin + 1; x <= xMax; ++x) {
        double y = a2 * x * x + a1 * x + a0;
        int yi = static_cast<int>(std::lround(y));

        bresenhamLineWorld(path, prevX, prevY, x, yi);

        prevX = x;
        prevY = yi;
    }
}


void App::drawCubic(std::vector<Pixel::Vertex>& path, double a3, double a2, double a1, double a0)
{
    path.clear();

    int xMin = -kWindowWidth / 2;
    int xMax =  kWindowWidth / 2 - 1;

    int prevX = xMin;
    int prevY = static_cast<int>(std::lround((((a3 * prevX + a2) * prevX + a1) * prevX + a0)));

    for (int x = xMin + 1; x <= xMax; ++x) {
        double y = ((a3 * x + a2) * x + a1) * x + a0;
        int yi = static_cast<int>(std::lround(y));

        bresenhamLineWorld(path, prevX, prevY, x, yi);

        prevX = x;
        prevY = yi;
    }
}


void App::bresenhamLineWorld(std::vector<Pixel::Vertex>& path, int x0, int y0, int x1, int y1)
{
    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);

    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;

    bool steep = dy > dx;
    if (steep) {
        std::swap(x0, y0);
        std::swap(x1, y1);
        std::swap(dx, dy);
        std::swap(sx, sy);
    }

    int err = 2 * dy - dx;
    int y = y0;
    int x = x0;

    for (int i = 0; i <= dx; ++i) {
        if (steep) pushWorld(path, y, x);
        else       pushWorld(path, x, y);

        if (err > 0) {
            y += sy;
            err -= 2 * dx;
        }
        err += 2 * dy;
        x += sx;
    }
}

void App::drawSuperquadric(std::vector<Pixel::Vertex>& path, double a, double b, double n)
{
    path.clear();
    if (a <= 0 || b <= 0 || n <= 0) return;

    // Parametric form of superellipse:
    // x(t) = a * sgn(cos t) * |cos t|^(2/n)
    // y(t) = b * sgn(sin t) * |sin t|^(2/n)

    const int samples = 4000;          // higher = smoother
    const double twoPi = 6.283185307179586;

    int prevX = 0, prevY = 0;
    bool hasPrev = false;

    for (int i = 0; i <= samples; ++i) {
        double t = twoPi * (double)i / (double)samples;

        double ct = std::cos(t);
        double st = std::sin(t);

        auto sgn = [](double v) { return (v < 0.0) ? -1.0 : 1.0; };

        double ex = 2.0 / n;

        double x = a * sgn(ct) * std::pow(std::abs(ct), ex);
        double y = b * sgn(st) * std::pow(std::abs(st), ex);

        int xi = (int)std::lround(x);
        int yi = (int)std::lround(y);

        if (!hasPrev) {
            prevX = xi; prevY = yi;
            hasPrev = true;
        } else {
            // connect successive points in WORLD space
            bresenhamLineWorld(path, prevX, prevY, xi, yi);
            prevX = xi; prevY = yi;
        }
    }
}
