#include <iostream>
#include <thread>
#include <Elementary.h>
#include <time.h>

#include "SkCanvas.h"
#include "SkColorSpace.h"
#include "SkSurface.h"
#include "SkTypes.h"

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

Evas_Object *view = nullptr;

void* pixels;
sk_sp<SkSurface> surface;
SkCanvas* canvas = nullptr;

std::vector<uint8_t> fileBytes;
int animationIndex = 0;
int stateMachineIndex = -1;

rive::SkiaFactory skiaFactory;

std::unique_ptr<rive::File> currentFile;
std::unique_ptr<rive::ArtboardInstance> artboardInstance;
std::unique_ptr<rive::Scene> currentScene;

std::vector<std::string> animationNames;
std::vector<std::string> stateMachineNames;

static Ecore_Animator *anim;

static Eina_Bool _do_animation(void *data, double pos)
{
    return ECORE_CALLBACK_RENEW;
}

static void createCanvas(unsigned int width, unsigned int height) {
    static constexpr auto BPP = 4;

    if (pixels) free(pixels);
    pixels = (void*) malloc (width * height * BPP);

    static constexpr SkAlphaType at = kPremul_SkAlphaType;
    const SkImageInfo info = SkImageInfo::MakeN32(width, height, at);

    surface = SkSurface::MakeRasterDirect(info, pixels, width * BPP);
    if (!surface) {
        fprintf(stderr, "Failed to create Skia surface\n");
    }

    canvas = surface->getCanvas();
}

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

    evas_object_image_pixels_dirty_set(view, EINA_TRUE);
    evas_render(view);
}

static void loadRivFile(const char* filename) {
    FILE* fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "file %s opening failure.\n", filename);
        return;
    }
    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    fileBytes.resize(size);
    if (fread(fileBytes.data(), 1, size, fp) != size) {
        fileBytes.clear();
        fprintf(stderr, "failed to read all of %s\n", filename);
        return;
    }
    printf("Loaded %s file succesfully.\n", filename);
    initAnimation(0);
}

static float getElapsedTime() {
    static clock_t staticClock = clock();
    clock_t t = clock();

    float elapsed = (float)(t - staticClock)/CLOCKS_PER_SEC;
    staticClock = t;
    return elapsed;
} 

void initSwView() {
    printf("[mszczeci][%s:%d][%s]\n",__FILE__, __LINE__, __func__);
    SkPaint paint;
    paint.setColor(SK_ColorGRAY);
    canvas->drawPaint(paint);

    loadRivFile("file.riv");
}

void drawSwView(void* data, Eo* obj) {
    if (currentScene) {
        const float elapsed = getElapsedTime();
        bool ret = currentScene->advanceAndApply(elapsed);

        clock_t t = clock();

        rive::SkiaRenderer renderer(canvas);
        renderer.save();

        const unsigned int width = WIDTH, height = HEIGHT;

        auto viewTransform = rive::computeAlignment(rive::Fit::contain,
                                                    rive::Alignment::center,
                                                    rive::AABB(0, 0, width, height),
                                                    currentScene->bounds());
        renderer.transform(viewTransform);

        currentScene->draw(&renderer);
        renderer.restore();

        t = clock() - t;
        double time_taken = ((double)t)/CLOCKS_PER_SEC;
        printf("Frame rendering time %f sec\n", time_taken);
    }
}

void transitSwCallback(Elm_Transit_Effect *effect, Elm_Transit* transit, double progress) {
    Eo* img = (Eo*) effect;
    evas_object_image_data_update_add(img, 0, 0, WIDTH, HEIGHT);
    evas_object_image_pixels_dirty_set(img, EINA_TRUE);
}

void win_del(void *data, Evas_Object *o, void *ev) {
   elm_exit();
}

int main(int argc, char **argv) {
    elm_init(argc, argv);

    Eo* win = elm_win_util_standard_add(NULL, "Rive-skia CPU Viewer");
    evas_object_smart_callback_add(win, "delete,request", win_del, 0);

    createCanvas(WIDTH, HEIGHT);

    view = evas_object_image_filled_add(evas_object_evas_get(win));
    evas_object_image_size_set(view, WIDTH, HEIGHT);
    evas_object_image_data_set(view, pixels);
    evas_object_image_pixels_get_callback_set(view, drawSwView, nullptr);
    evas_object_image_pixels_dirty_set(view, EINA_TRUE);
    evas_object_image_data_update_add(view, 0, 0, WIDTH, HEIGHT);
    evas_object_size_hint_weight_set(view, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_show(view);

    elm_win_resize_object_add(win, view);
    evas_object_geometry_set(win, 0, 0, WIDTH, HEIGHT);
    evas_object_show(win);

    Elm_Transit *transit = elm_transit_add();
    elm_transit_effect_add(transit, transitSwCallback, view, nullptr);
    elm_transit_duration_set(transit, 1);
    elm_transit_repeat_times_set(transit, -1);
    elm_transit_auto_reverse_set(transit, EINA_TRUE);
    elm_transit_go(transit);

    initSwView();

    elm_run();
    elm_shutdown();

    return 0;
}