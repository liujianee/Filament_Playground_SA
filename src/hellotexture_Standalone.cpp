#include <iostream>
#include <SDL.h>
#include <SDL_opengl.h>
#include <stdio.h>

#include <atomic>
#include <fstream>
#include <iomanip>
#include <map>
#include <memory>
#include <string>
#include <sstream>
#include <utility>
#include <vector>

//#include <getopt/getopt.h>

#include <filament/Camera.h>
#include <filament/Engine.h>
#include <filament/LightManager.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/RenderableManager.h>
#include <filament/Scene.h>
#include <filament/Texture.h>
#include <filament/TextureSampler.h>
#include <filament/TransformManager.h>
#include <filament/Skybox.h>
#include <filament/View.h>
#include <filament/Viewport.h>
#include <filament/LightManager.h>
#include <filament/Renderer.h>
#include <filament/IndirectLight.h>

#include <filamat/MaterialBuilder.h>

// copied from FilamentAPP/include
#include <filament/NativeWindowHelper.h>
#include <filament/IBL.h>

//#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <utils/EntityManager.h>
#include <utils/Panic.h>
#include <utils/Path.h>

#include <filameshio/MeshReader.h>

#include <filament/FilamentAPI.h>
#include <math/norm.h>
#include <math/mat3.h>
#include <math/mat4.h>
#include <math/vec4.h>

#include <cmath>
//#include "generated/resources/resources.h"

using namespace filament::math;
using namespace filament;
using namespace filamesh;
using namespace filamat;
using namespace utils;
using namespace math;
using namespace std;

using utils::Entity;
using utils::EntityManager;

// static std::vector<Path> g_filenames;

static MeshReader::MaterialRegistry g_materialInstances;
//static std::vector<MeshReader::Mesh> g_meshes;
static MeshReader::Mesh mMesh;
static const Material* g_material;
static Entity g_light;
static std::map<std::string, Texture*> g_maps;

static constexpr uint8_t RESOURCES_TEXTUREDLIT_DATA[] = {
#include "../resource/materials/texturedLit.inc"
};

const char* basecolor_f = "../resource/textures/basecolor.png";
const char* metallic_f = "../resource/textures/metallic.png";
const char* normal_f = "../resource/textures/normal.png";
const char* roughness_f = "../resource/textures/roughness.png";
const char* ao_f = "../resource/textures/ao.png";

static const std::string model_f = "../resource/filamesh/suzanne.filamesh";


void initSDL() {
    ASSERT_POSTCONDITION(SDL_Init(SDL_INIT_EVENTS) == 0, "SDL_Init Failure");
}


SDL_Window* createSDLwindow() {

    const uint32_t windowFlags =
            SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
    SDL_Window* win =
            SDL_CreateWindow("Hello Suzanne!", 100, 100, 1024, 640, windowFlags);
    if (win == nullptr) {
        std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return NULL;
    }

    return win;
}



// function to display 4x4 matrix
void display_4x4( string tag, mat4f m4 )
{
    cout << tag << '\n';
    for (int col = 0; col < 4; ++col) {
        cout << "| ";
        for (int row = 0; row < 4; ++row) {
            cout << m4[row][col] << '\t';
        }
        cout << '\n';
    }
    cout << '\n';
}


Texture* loadMap(Engine* engine, const char* name, bool sRGB = true) {
    Path path(name);
    if (path.exists()) {
        int w, h, n;
        unsigned char* data = stbi_load(path.getAbsolutePath().c_str(), &w, &h, &n, 3);
        if (data != nullptr) {
            Texture* map = Texture::Builder()
                    .width(uint32_t(w))
                    .height(uint32_t(h))
                    .levels(0xff)
                    .format(sRGB ? Texture::InternalFormat::SRGB8 : Texture::InternalFormat::RGB8)
                    .build(*engine);
            Texture::PixelBufferDescriptor buffer(data, size_t(w * h * 3),
                    Texture::Format::RGB, Texture::Type::UBYTE,
                    (Texture::PixelBufferDescriptor::Callback) &stbi_image_free);
            map->setImage(*engine, 0, std::move(buffer));
            map->generateMipmaps(*engine);
            g_maps[name] = map;
            return map;
        } else {
            std::cout << "The map " << path << " could not be loaded" << std::endl;
        }
    } else {
        std::cout << "The map " << path << " does not exist" << std::endl;
    }
    return nullptr;
}



//static void setup(Engine* engine, View* view, Scene* scene) {
MeshReader::Mesh setup(Engine* engine, View* view, Scene* scene) {
    Texture* normal = loadMap(engine, normal_f, false);
    Texture* basecolor = loadMap(engine, basecolor_f);
    Texture* roughness = loadMap(engine, roughness_f, false);
    Texture* metallic = loadMap(engine, metallic_f, false);
    Texture* ao = loadMap(engine, ao_f, false);

    if (!basecolor || !normal || !roughness) {
        std::cout << "Need basecolor.png, normal.png and roughness.png" << std::endl;
        //return;
    }

    MaterialBuilder::init();

    // Instantiate material.
    g_material = Material::Builder()
            .package((void*) RESOURCES_TEXTUREDLIT_DATA, sizeof(RESOURCES_TEXTUREDLIT_DATA)).build(*engine);
    const utils::CString defaultMaterialName("DefaultMaterial");
    g_materialInstances.registerMaterialInstance(defaultMaterialName, g_material->createInstance());

    TextureSampler sampler(TextureSampler::MinFilter::LINEAR_MIPMAP_LINEAR,
            TextureSampler::MagFilter::LINEAR, TextureSampler::WrapMode::REPEAT);
    //sampler.setAnisotropy(8.0f);
    g_materialInstances.getMaterialInstance(defaultMaterialName)->setParameter("normal", normal, sampler);
    g_materialInstances.getMaterialInstance(defaultMaterialName)->setParameter("albedo", basecolor, sampler);
    g_materialInstances.getMaterialInstance(defaultMaterialName)->setParameter("roughness", roughness, sampler);
    g_materialInstances.getMaterialInstance(defaultMaterialName)->setParameter("metallic", metallic, sampler);
    g_materialInstances.getMaterialInstance(defaultMaterialName)->setParameter("ao", ao, sampler);

//    auto ibl = FilamentApp::get().getIBL()->getIndirectLight();
//    ibl->setIntensity(100000);
//    ibl->setRotation(mat3f::rotation(0.5f, float3{ 0, 1, 0 }));

    auto& tcm = engine->getTransformManager();
    
    std::cout << "loading mesh ..." << std::endl;
    Path filename(model_f);
    MeshReader::Mesh mesh = MeshReader::loadMeshFromFile(engine, filename, g_materialInstances);
    if (mesh.renderable) {
        std::cout << "mesh loaded successfully!" << std::endl;
        auto ei = tcm.getInstance(mesh.renderable);
        tcm.setTransform(ei, mat4f{ mat3f(1.0), float3(0.0f, 0.0f, -4.0f) } *
                             tcm.getWorldTransform(ei));
        scene->addEntity(mesh.renderable);
    }
    std::cout << "setting lights ..." << std::endl;
    g_light = EntityManager::get().create();
    LightManager::Builder(LightManager::Type::DIRECTIONAL)
            .color(Color::toLinear<ACCURATE>({0.98f, 0.92f, 0.89f}))
            .intensity(110000)
            .direction({0.6, -1, -0.8})
            .build(*engine, g_light);
    scene->addEntity(g_light);

    return mesh;
}


int main() {
    std::string iblDirectory = "../resource/default_env/";

    SDL_Window* window = createSDLwindow();
    Engine* engine = Engine::create();

    void* nativeWindow = getNativeWindow(window);
    void* nativeSwapChain = nativeWindow;
    prepareNativeWindow(window);

    void* metalLayer = nullptr;
    metalLayer = setUpMetalLayer(nativeWindow);
    nativeSwapChain = metalLayer;

    std::unique_ptr<IBL> mIBL;
    mIBL = std::make_unique<IBL>(*engine);
    Path iblPath(iblDirectory);
    if (!iblPath.isDirectory()) {
        mIBL->loadFromEquirect(iblPath);
    }
    else {
        mIBL->loadFromDirectory(iblPath);
    }

    SwapChain* swapChain = engine->createSwapChain(nativeSwapChain);
    Renderer* renderer = engine->createRenderer();

    TransformManager& tcm = engine->getTransformManager();
    auto& rcm = engine->getRenderableManager();

    Entity my_camera = utils::EntityManager::get().create();
    Camera* camera = engine->createCamera(my_camera);
    View* view = engine->createView();
    Scene* scene = engine->createScene();

    if (mIBL != nullptr) {
        mIBL->getSkybox()->setLayerMask(0x7, 0x4);
        scene->setSkybox(mIBL->getSkybox());
        scene->setIndirectLight(mIBL->getIndirectLight());
    }
    
    IBL* my_IBL = mIBL.get();
    auto ibl = my_IBL->getIndirectLight();;
    ibl->setIntensity(100000);
    ibl->setRotation(mat3f::rotation(0.5f, float3{ 0, 1, 0 }));

    view->setCamera(camera);

    uint32_t w, h;
    SDL_GL_GetDrawableSize(window, (int*) &w, (int*) &h);

    camera->setProjection(45.0, double(w) / h, 0.1, 50, Camera::Fov::VERTICAL);
    view->setViewport({ 0, 0, w, h });

    view->setScene(scene);
    
    Skybox* skybox = Skybox::Builder().color({0.1, 0.125, 0.25, 1.0}).build(*engine);
//    scene->setSkybox(skybox);     // uncomment this line if color background is needed
    view->setPostProcessingEnabled(false);
    
    view->setVisibleLayers(0x4, 0x4);

    mMesh = setup(engine, view, scene);

//    mat4f origTransform = math::mat4f::translation(float3{ 0, 0, -4 }) * mat4f::rotation(0.0 * M_PI, float3{ 1, 0, 0 });
    auto cube_ti = tcm.getInstance(mMesh.renderable);
//    rcm.setCastShadows(rcm.getInstance(mMesh.renderable), false);
//    tcm.setTransform(cube_ti, origTransform);
    mat4f origTransform = tcm.getWorldTransform(cube_ti);

    scene->addEntity(mMesh.renderable);

    constexpr double speed = 100.0;
    bool run = true;
    double g_discoAngle = 0;
    double g_discoAngularSpeed = 1;
    double lastTime = 0;

    float3 pos = 0;

    while (run) {
        // beginFrame() returns false if we need to skip a frame
        if (renderer->beginFrame(swapChain)) {
            // for each View
            renderer->render(view);
            renderer->endFrame();
        }
        double now = (double) SDL_GetPerformanceCounter() / SDL_GetPerformanceFrequency();
        double dT = now - lastTime;
        SDL_Event event;
        int numEvents = 0;
        while (SDL_PollEvent(&event) && numEvents < 16) {

            switch (event.type) {
                case SDL_QUIT:
                    run = false;
                    break;

                case SDL_KEYDOWN: {
                    switch ((int) event.key.keysym.scancode) {
                        case SDL_SCANCODE_ESCAPE: {
                            run = false;
                            break;
                        }
                        case SDL_SCANCODE_W: {
                            pos.y += dT * speed;
                            break;
                        }
                        case SDL_SCANCODE_A: {

                            pos.x -= dT * speed;
                            break;
                        }
                        case SDL_SCANCODE_S: {

                            pos.y -= dT * speed;
                            break;
                        }
                        case SDL_SCANCODE_D: {

                            pos.x += dT * speed;
                            break;
                        }
                    }
                    break;
                }
            }
            numEvents++;
        }

        SDL_Delay(16);
        camera->setModelMatrix(mat4f::translation(pos));
        // mat4f transform = mat4f::translation(float3{ 0, 0, -4 }) * mat4f::rotation(g_discoAngle, float3{ 0, 0, 1 });
        mat4f transform = mat4f::rotation(g_discoAngle, float3{ 0, 0, 1 });
        transform *= origTransform;
        display_4x4("display transform :", transform);
        tcm.setTransform(cube_ti, transform);
        g_discoAngle += g_discoAngularSpeed * dT;

        lastTime = now;
    }

    engine->destroy(my_camera);
//    engine->destroy(skybox);

    return 0;
}

