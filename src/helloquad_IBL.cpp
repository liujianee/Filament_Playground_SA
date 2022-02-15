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

// copied from FilamentAPP/include
#include <filament/NativeWindowHelper.h>
#include <filament/IBL.h>

#include <utils/EntityManager.h>
#include <utils/Panic.h>
#include <utils/Path.h>

#include <filament/FilamentAPI.h>
#include <math/norm.h>

#include <cmath>

#define WIDTH 1024
#define HEIGHT 640


using namespace filament;
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


int main()
{
    std::string iblDirectory = "../resource/default_env/";

    const static uint32_t indices[] = { 0, 1, 2, 2, 3, 0 };

    float3 vertices[] = {
        { -10, 0, -10 },
        { -10, 0, 10 },
        { 10, 0, 10 },
        { 10, 0, -10 },
    };

    short4 tbn = math::packSnorm16(
            mat3f::packTangentFrame(math::mat3f{ float3{ 1.0f, 0.0f, 0.0f },
                                            float3{ 0.0f, 0.0f, 1.0f },
                                            float3{ 0.0f, 1.0f, 0.0f } })
                    .xyzw);

    const static math::short4 normals[]{ tbn, tbn, tbn, tbn };

    SDL_Window* window = createSDLwindow();
    Engine* engine = Engine::create();

    void* nativeWindow = getNativeWindow(window);
    void* nativeSwapChain = nativeWindow;
    prepareNativeWindow(window);

    void* metalLayer = nullptr;
    metalLayer = setUpMetalLayer(nativeWindow);
    nativeSwapChain = metalLayer;


    SwapChain* swapChain = engine->createSwapChain(nativeSwapChain);
    // SwapChain* swapChain = engine->createSwapChain(getNativeWindow(window));
    Renderer* renderer = engine->createRenderer();

    TransformManager& tcm = engine->getTransformManager();

    Entity my_camera = utils::EntityManager::get().create();
    Camera* camera = engine->createCamera(my_camera);
    View* view = engine->createView();
    Scene* scene = engine->createScene();

    // load IBL files
    std::unique_ptr<IBL> mIBL;
    mIBL = std::make_unique<IBL>(*engine);
    Path iblPath(iblDirectory);
    if (!iblPath.isDirectory()) {
        mIBL->loadFromEquirect(iblPath);
    }
    else {
        mIBL->loadFromDirectory(iblPath);
    }

    // set up IBL
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
    
//    Skybox* skybox = Skybox::Builder().color({0.1, 0.125, 0.25, 1.0}).build(*engine);
    Skybox* skybox = Skybox::Builder().color({0.0, 1.0, 0.0, 1.0}).build(*engine);
//    scene->setSkybox(skybox);
    view->setPostProcessingEnabled(false);
    
    view->setVisibleLayers(0x4, 0x4);

    VertexBuffer* vertexBuffer =
            VertexBuffer::Builder()
                    .vertexCount(4)
                    .bufferCount(2)
                    .attribute(VertexAttribute::POSITION, 0,
                            VertexBuffer::AttributeType::FLOAT3)
                    .attribute(VertexAttribute::TANGENTS, 1,
                            VertexBuffer::AttributeType::SHORT4)
                    .normalized(VertexAttribute::TANGENTS)
                    .build(*engine);

    vertexBuffer->setBufferAt(
            *engine, 0,
            VertexBuffer::BufferDescriptor(vertices, vertexBuffer->getVertexCount() * sizeof(vertices[0])));
    vertexBuffer->setBufferAt(
            *engine, 1,
            VertexBuffer::BufferDescriptor(normals, vertexBuffer->getVertexCount() * sizeof(normals[0])));


    IndexBuffer* indexBuffer =
            IndexBuffer::Builder().indexCount(6).build(*engine);

    indexBuffer->setBuffer(
            *engine, IndexBuffer::BufferDescriptor(indices, indexBuffer->getIndexCount() * sizeof(uint32_t)));


    Material* material = Material::Builder()
                                 .package((void*) BAKED_MATERIAL_PACKAGE, sizeof(BAKED_MATERIAL_PACKAGE))
                                 .build(*engine);
    material->setDefaultParameter("baseColor", RgbType::LINEAR, float3{ 0.8, 0, 0 });
    material->setDefaultParameter("metallic", 0.0f);
    material->setDefaultParameter("roughness", 0.4f);
    material->setDefaultParameter("reflectance", 0.5f);

    MaterialInstance* materialInstance = material->createInstance();

    Entity renderable = EntityManager::get().create();

    Entity light = EntityManager::get().create();

//    LightManager::Builder(LightManager::Type::SUN)
//            .color(Color::toLinear<ACCURATE>(sRGBColor(0.98f, 0.92f, 0.89f)))
//            .intensity(110000)
//            .direction({ 0.7, -1, -0.8 })
//            .sunAngularRadius(1.9f)
//            .castShadows(true)
//            .build(*engine, light);

//    scene->addEntity(light);

    // build a quad
    RenderableManager::Builder(1)
            .boundingBox({ { -1, -1, -1 }, { 1, 1, 1 } })
            .material(0, materialInstance)
            .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, vertexBuffer,
                    indexBuffer, 0, 6)
            .culling(false)
            .build(*engine, renderable);


    scene->addEntity(renderable);
    mat4f origTransform = math::mat4f::translation(float3{ 0, -1, -100 }) * mat4f::rotation(0.5 * M_PI, float3{ 1, 0, 0 });
    mat4 camTrans = camera->getModelMatrix();
    tcm.setTransform(tcm.getInstance(renderable), origTransform);


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
                            pos.z += dT * speed;
                            break;
                        }
                        case SDL_SCANCODE_A: {

                            pos.x -= dT * speed;
                            break;
                        }
                        case SDL_SCANCODE_S: {

                            pos.z -= dT * speed;
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
        // float3 transform = float3(0,2,-4)
        mat4f transform = mat4f::translation(float3{ 0, -1, -50 }) * mat4f::rotation(g_discoAngle, float3{ 0, 0, 1 });
        transform *= origTransform;
        tcm.setTransform(tcm.getInstance(renderable), transform);
        g_discoAngle += g_discoAngularSpeed * dT;

        lastTime = now;
    }

    engine->destroy(my_camera);
//    engine->destroy(skybox);

    return 0;
}








