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

#include <filament/Camera.h>
#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/RenderableManager.h>
#include <filament/Scene.h>
#include <filament/Skybox.h>
#include <filament/TransformManager.h>
#include <filament/VertexBuffer.h>
#include <filament/View.h>
#include <filament/Viewport.h>
#include <filament/LightManager.h>
#include <filament/Renderer.h>
#include <filament/IndirectLight.h>

#include <filamat/MaterialBuilder.h>
#include <geometry/SurfaceOrientation.h>

// copied from FilamentAPP/include
#include <filament/NativeWindowHelper.h>
#include <filament/IBL.h>

#include <utils/EntityManager.h>
#include <utils/Panic.h>
#include <utils/Path.h>

#include <filameshio/MeshReader.h>

#include <filament/FilamentAPI.h>
#include <math/norm.h>
#include <math/mat3.h>

#include <cmath>
//#include "generated/resources/resources.h"
//#include "generated/resources/monkey.h"

#define WIDTH 1024
#define HEIGHT 640

#define RELATIVE_ASSET_PATH ".."

using namespace filament;
using namespace filamesh;

using namespace utils;
using namespace math;
using namespace std;

using utils::Entity;
using utils::EntityManager;

static constexpr uint8_t BAKED_MATERIAL_PACKAGE[] = {
#include "../resource/materials/aiDefaultMat.inc"
};


void initSDL() {
    ASSERT_POSTCONDITION(SDL_Init(SDL_INIT_EVENTS) == 0, "SDL_Init Failure");
}


SDL_Window* createSDLwindow() {

    const uint32_t windowFlags =
            SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
    SDL_Window* win =
            SDL_CreateWindow("Hello World!", 100, 100, 1024, 640, windowFlags);
    if (win == nullptr) {
        std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return NULL;
    }

    return win;
}

const utils::Path& getRootAssetsPath() {
    static const utils::Path root = utils::Path::getCurrentExecutable().getParent() + RELATIVE_ASSET_PATH;
    return root;
}

static const char* IBL_FOLDER = "default_env";


/* -------------------------------------------------------------- */
class Monkey {
public:
    Monkey() = default;
    ~Monkey();
    int init(filament::Engine* engine);
    int shutdown();


public:
    MeshReader::Mesh mesh;
    const filament::Material* material = nullptr;
    filament::MaterialInstance* material_instance = nullptr;
    const Material* g_material;
    MeshReader::MaterialRegistry g_materialInstances;
    std::string filename = "../resource/filamesh/suzanne.filamesh";
};

Monkey::~Monkey() {
    shutdown();
}

/*
   Stored as Vertex(pos, norm, uv)
   pos: offset 0,
   norm: offset 12
   uv: offset 24
   vertex stride: 8 * 4 = 32 bytes
*/
int Monkey::init(filament::Engine* engine) {
    
    /* create material */
    if (nullptr == material_instance) {
    
      std::string shader = R"SHADER(
        void material(inout MaterialInputs material) {
           prepareMaterial(material);
           material.baseColor.rgb = float3(1.0, 1.0, 1.0);
           material.metallic = 1.0;
           material.roughness = 0.0;
        }
      )SHADER";
    
      filamat::MaterialBuilder::init();
      filamat::MaterialBuilder mat_builder = filamat::MaterialBuilder();
      mat_builder.name("DefaultMaterial");
      mat_builder.material(shader.c_str());
      mat_builder.doubleSided(true);
      mat_builder.shading(filament::Shading::LIT);
      mat_builder.targetApi(filamat::MaterialBuilder::TargetApi::ALL);
    
    
      // this is added
      utils::JobSystem* jobsystem;
      jobsystem = &engine->getJobSystem();
      filamat::Package pkg = mat_builder.build(*jobsystem);
      filament::Material::Builder fil_mat_builder = filament::Material::Builder();
      fil_mat_builder.package(pkg.getData(), pkg.getSize());
    
      // material = fil_mat_builder.build(*engine);
      // material_instance = material->createInstance();
      g_material = fil_mat_builder.build(*engine);
      const utils::CString defaultMaterialName("DefaultMaterial");
      g_materialInstances.registerMaterialInstance(defaultMaterialName, g_material->createInstance());
    }
    
    // mesh = MeshReader::loadMeshFromBuffer(engine, MONKEY_SUZANNE_DATA, nullptr, nullptr, material_instance);
    mesh = MeshReader::loadMeshFromFile(engine, filename, g_materialInstances);
    return 0;
}

int Monkey::shutdown() {
    int r = 0;
    return r;
}

/* -------------------------------------------------------------- */


int main()
{
    Monkey myMonkey;

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


//     //.package(RESOURCES_BAKEDCOLOR_DATA, RESOURCES_BAKEDCOLOR_SIZE)
//     //.package((void*) BAKED_MATERIAL_PACKAGE, sizeof(BAKED_MATERIAL_PACKAGE))
//     Material* material = Material::Builder()
//                                  .package((void*) BAKED_MATERIAL_PACKAGE, sizeof(BAKED_MATERIAL_PACKAGE))
//                                  .build(*engine);
//     material->setDefaultParameter("baseColor", RgbType::LINEAR, float3{ 0.8, 0, 0 });
//     material->setDefaultParameter("metallic", 0.0f);
//     material->setDefaultParameter("roughness", 0.4f);
//     material->setDefaultParameter("reflectance", 0.5f);
// 
//     MaterialInstance* materialInstance = material->createInstance();
// //    MaterialInstance* materialInstance = material->getDefaultInstance();
// 
//     Entity renderable = EntityManager::get().create();
// 
// //    Entity light = EntityManager::get().create();
// 
// 
// //    LightManager::Builder(LightManager::Type::SUN)
// //            .color(Color::toLinear<ACCURATE>(sRGBColor(0.98f, 0.92f, 0.89f)))
// //            .intensity(110000)
// //            .direction({ 0.7, -1, -0.8 })
// //            .sunAngularRadius(1.9f)
// //            .castShadows(true)
// //            .build(*engine, light);
// 
// //    scene->addEntity(light);

    mat4f origTransform = math::mat4f::translation(float3{ 0, 0, -4 }) * mat4f::rotation(0.2 * M_PI, float3{ 1, 0, 0 });
//     mat4 camTrans = camera->getModelMatrix();
//     tcm.setTransform(tcm.getInstance(renderable), origTransform);

    myMonkey.init(engine);

    auto cube_ti = tcm.getInstance(myMonkey.mesh.renderable);
    rcm.setCastShadows(rcm.getInstance(myMonkey.mesh.renderable), false);
    tcm.setTransform(cube_ti, origTransform);

    scene->addEntity(myMonkey.mesh.renderable);


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
        mat4f transform = mat4f::translation(float3{ 0, 0, -4 }) * mat4f::rotation(g_discoAngle, float3{ 0, 0, 1 });
        transform *= origTransform;
        tcm.setTransform(cube_ti, transform);
        g_discoAngle += g_discoAngularSpeed * dT;

        lastTime = now;
    }

    engine->destroy(my_camera);
//    engine->destroy(skybox);

    return 0;
}

