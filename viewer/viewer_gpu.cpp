#include <cstdio>
#include <cmath>

#define SK_GL
#include <GLFW/glfw3.h>

#include "GrBackendSurface.h"
#include "GrDirectContext.h"
#include "SkCanvas.h"
#include "SkColorSpace.h"
#include "SkSurface.h"
#include "SkTypes.h"
#include "gl/GrGLInterface.h"

#include "rive/animation/linear_animation_instance.hpp"
#include "rive/animation/state_machine_instance.hpp"
#include "rive/animation/state_machine_input_instance.hpp"
#include "rive/animation/state_machine_number.hpp"
#include "rive/animation/state_machine_bool.hpp"
#include "rive/animation/state_machine_trigger.hpp"
#include "rive/artboard.hpp"
#include "rive/file.hpp"
#include "rive/layout.hpp"
#include "rive/math/aabb.hpp"
#include "skia_factory.hpp"
#include "skia_renderer.hpp"

#define WIDTH 1280
#define HEIGHT 720

rive::SkiaFactory skiaFactory;

std::string filename;
std::unique_ptr<rive::File> currentFile;
std::unique_ptr<rive::ArtboardInstance> artboardInstance;
std::unique_ptr<rive::Scene> currentScene;

std::vector<std::string> animationNames;
std::vector<std::string> stateMachineNames;

#include <time.h>
double GetSecondsToday() {
    time_t m_time;
    time(&m_time);
    struct tm tstruct;
    gmtime_r(&m_time, &tstruct);

    int hours = tstruct.tm_hour - 4;
    if (hours < 0) {
        hours += 12;
    } else if (hours >= 12) {
        hours -= 12;
    }

    auto secs = (double)hours * 60 * 60 +
                (double)tstruct.tm_min * 60 +
                (double)tstruct.tm_sec;
//    printf("%d %d %d\n", tstruct.tm_sec, tstruct.tm_min, hours);
//    printf("%g %g %g\n", secs, secs/60, secs/60/60);
    return secs;
}

std::vector<uint8_t> fileBytes;

int animationIndex = 0;
int stateMachineIndex = -1;

static void loadNames(const rive::Artboard* ab) {
    animationNames.clear();
    stateMachineNames.clear();
    if (ab) {
        for (size_t i = 0; i < ab->animationCount(); ++i) {
            animationNames.push_back(ab->animationNameAt(i));
        }
        for (size_t i = 0; i < ab->stateMachineCount(); ++i) {
            stateMachineNames.push_back(ab->stateMachineNameAt(i));
        }
    }
}

static void initAnimation(int index) {
    animationIndex = index;
    stateMachineIndex = -1;
    assert(fileBytes.size() != 0);
    auto file = rive::File::import(rive::toSpan(fileBytes), &skiaFactory);
    if (!file) {
        fileBytes.clear();
        fprintf(stderr, "failed to import file\n");
        return;
    }
    currentScene = nullptr;
    artboardInstance = nullptr;

    currentFile = std::move(file);
    artboardInstance = currentFile->artboardDefault();
    artboardInstance->advance(0.0f);
    loadNames(artboardInstance.get());

    if (index >= 0 && index < artboardInstance->animationCount()) {
        currentScene = artboardInstance->animationAt(index);
        currentScene->inputCount();
    }
}


rive::Mat2D gInverseViewTransform;
rive::Vec2D lastWorldMouse;
static void glfwCursorPosCallback(GLFWwindow* window, double x, double y) {
    float xscale, yscale;
    glfwGetWindowContentScale(window, &xscale, &yscale);
    lastWorldMouse = gInverseViewTransform * rive::Vec2D(x * xscale, y * yscale);
    if (currentScene) {
        currentScene->pointerMove(lastWorldMouse);
    }
}
static void glfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (currentScene) {
        switch (action) {
            case GLFW_PRESS:
                currentScene->pointerDown(lastWorldMouse);
                break;
            case GLFW_RELEASE:
                currentScene->pointerUp(lastWorldMouse);
                break;
        }
    }
}

static void glfwErrorCallback(int error, const char* description) {
    puts(description);
}

static void glfwDropCallback(GLFWwindow* window, int count, const char** paths) {
    filename = paths[0];
    FILE* fp = fopen(filename.c_str(), "rb");
    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    fileBytes.resize(size);
    if (fread(fileBytes.data(), 1, size, fp) != size) {
        fileBytes.clear();
        fprintf(stderr, "failed to read all of %s\n", filename.c_str());
        return;
    }
    printf("Loaded %s file succesfully.\n", filename.c_str());
    initAnimation(0);
}

int main() {
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize glfw.\n");
        return 1;
    }
    glfwSetErrorCallback(glfwErrorCallback);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Rive-skia GPU Viewer", NULL, NULL);
    if (window == nullptr) {
        fprintf(stderr, "Failed to make window or GL.\n");
        glfwTerminate();
        return 1;
    }

    glfwSetDropCallback(window, glfwDropCallback);
    glfwSetCursorPosCallback(window, glfwCursorPosCallback);
    glfwSetMouseButtonCallback(window, glfwMouseButtonCallback);
    glfwMakeContextCurrent(window);

    // Enable VSYNC.
    glfwSwapInterval(1);

    // Setup Skia
    GrContextOptions options;
    sk_sp<GrDirectContext> context = GrDirectContext::MakeGL(nullptr, options);
    GrGLFramebufferInfo framebufferInfo;
    framebufferInfo.fFBOID = 0;
    framebufferInfo.fFormat = GL_RGBA8;

    sk_sp<SkSurface> surface;
    SkCanvas* canvas = nullptr;
    
    int width = 0, height = 0;
    int lastScreenWidth = 0, lastScreenHeight = 0;

    double lastTime = glfwGetTime();

    static constexpr SkAlphaType at = kPremul_SkAlphaType;
    const SkImageInfo info = SkImageInfo::MakeN32(WIDTH, HEIGHT, at);

    printf("Viewer loaded. Drag and drop .riv file to load.\n");

    while (!glfwWindowShouldClose(window)) {
        glfwGetFramebufferSize(window, &width, &height);

        if (!surface || width != lastScreenWidth || height != lastScreenHeight) {
            lastScreenWidth = width;
            lastScreenHeight = height;

            SkColorType colorType = kRGBA_8888_SkColorType;

            GrBackendRenderTarget backendRenderTarget(width,
                                                      height,
                                                      0, // sample count
                                                      0, // stencil bits
                                                      framebufferInfo);

            surface = SkSurface::MakeFromBackendRenderTarget(context.get(),
                                                             backendRenderTarget,
                                                             kBottomLeft_GrSurfaceOrigin,
                                                             colorType,
                                                             nullptr,
                                                             nullptr);

            if (!surface) {
                fprintf(stderr, "Failed to create Skia surface\n");
                return 1;
            }

            canvas = surface->getCanvas();
        }

        double time = glfwGetTime();
        float elapsed = (float)(time - lastTime);
        lastTime = time;

        SkPaint paint;
        paint.setColor(SK_ColorGRAY);
        canvas->drawPaint(paint);

        if (currentScene) {
            if (auto num = currentScene->getNumber("isTime")) {
                num->value(GetSecondsToday()/60/60);
            }

            currentScene->advanceAndApply(elapsed);

            rive::SkiaRenderer renderer(canvas);
            renderer.save();

            auto viewTransform = rive::computeAlignment(rive::Fit::contain,
                                                        rive::Alignment::center,
                                                        rive::AABB(0, 0, width, height),
                                                        currentScene->bounds());
            renderer.transform(viewTransform);
            gInverseViewTransform = viewTransform.invertOrIdentity();

            currentScene->draw(&renderer);
            renderer.restore();
        }
        context->flush();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}