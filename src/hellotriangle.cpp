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

#include <filamat/MaterialBuilder.h>

// copied from FilamentAPP/include
#include <filament/NativeWindowHelper.h>

#include <utils/EntityManager.h>
#include <utils/Panic.h>

#include <filament/FilamentAPI.h>
#include <math/norm.h>

#include <cmath>

#define WIDTH 1600
#define HEIGHT 1000


using namespace filament;
using namespace utils;
using namespace math;
using namespace std;

using utils::Entity;
using utils::EntityManager;


// int main(int argc, char** argv)
// {
//     Engine *engine = Engine::create();
//     engine->destroy(&engine);
//     return 0;
// }
//

//SDL_Window* mWindow = nullptr;


// static std::string g_materialPath = "/Users/jian.liu/Tutorials/Filament/web_tutorials/redball/plastic_metal.filamat";
// static std::vector<char> g_materialBuffer;

// static Material* g_material = nullptr;
// static MaterialInstance* g_materialInstance = nullptr;

static constexpr uint8_t BAKED_MATERIAL_PACKAGE[] = {
#include "../materials/bakedColor.inc"
};


// static std::ifstream::pos_type getFileSize(const char* filename) {
//     std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
//     return in.tellg();
// }
// 
// static void readMaterial(Engine* engine) {
//     long fileSize = static_cast<long>(getFileSize(g_materialPath.c_str()));
//     if (fileSize <= 0) {
//         return;
//     }
// 
//     std::ifstream in(g_materialPath.c_str(), std::ifstream::in);
//     if (in.is_open()) {
//         g_materialBuffer.reserve(static_cast<unsigned long>(fileSize));
//         if (in.read(g_materialBuffer.data(), fileSize)) {
//             g_material = Material::Builder()
//                     .package((void*) g_materialBuffer.data(), (size_t) fileSize)
//                     .build(*engine);
//             //g_materialInstance = g_material->createInstance();
//         }
//     }
// }



void initSDL() {
    ASSERT_POSTCONDITION(SDL_Init(SDL_INIT_EVENTS) == 0, "SDL_Init Failure");
}

SDL_Window* APP_Window(std::string title, size_t w, size_t h) {
    const int x = SDL_WINDOWPOS_CENTERED;
    const int y = SDL_WINDOWPOS_CENTERED;
    const uint32_t windowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE;

    SDL_Window* window = SDL_CreateWindow(title.c_str(), x, y, (int) w, (int) h, windowFlags);
    //SDL_Window* window = SDL_CreateWindow(title.c_str(), 100, 100, 1920, 1080, windowFlags);
    return window;
}


struct Vertex {
    filament::math::float2 position;
    uint32_t color;
};

static const Vertex TRIANGLE_VERTICES[3] = {
    {{1, 0}, 0xffff0000u},
    {{cos(M_PI * 2 / 3), sin(M_PI * 2 / 3)}, 0xff00ff00u},
    {{cos(M_PI * 4 / 3), sin(M_PI * 4 / 3)}, 0xff0000ffu},
};

static constexpr uint16_t TRIANGLE_INDICES[3] = { 0, 1, 2 };


int main()
{
//    static_assert(sizeof(Vertex) == 12, "Strange vertex size.");

    const static uint32_t indices[] = { 0, 1, 2, 2, 3, 0 };

    float3 vertices[] = {
        { -1, 0, -10 },
        { -1, 0, 10 },
        { 1, 0, 10 },
        { 1, 0, -10 },
    };

    short4 tbn = math::packSnorm16(
            mat3f::packTangentFrame(math::mat3f{ float3{ 1.0f, 0.0f, 0.0f },
                                            float3{ 0.0f, 0.0f, 1.0f },
                                            float3{ 0.0f, 1.0f, 0.0f } })
                    .xyzw);

    const static math::short4 normals[]{ tbn, tbn, tbn, tbn };


    SDL_Window* mWindow = APP_Window("JIAN Filament", WIDTH, HEIGHT);
    Engine* mEngine = Engine::create();

    // Engine* mEngine = nullptr;
    // mEngine = Engine::create(Engine::Backend::METAL);

    void* nativeWindow = getNativeWindow(mWindow);
    void* nativeSwapChain = nativeWindow;
    prepareNativeWindow(mWindow);

    void* metalLayer = nullptr;
    metalLayer = setUpMetalLayer(nativeWindow);
    nativeSwapChain = metalLayer;

    // Engine::Backend mBackend;

    // SwapChain* mSwapChain = nullptr;
    SwapChain* mSwapChain = mEngine->createSwapChain(nativeSwapChain);

    // Renderer* mRenderer = nullptr;
    Renderer* mRenderer = mEngine->createRenderer();

    TransformManager& tcm = mEngine->getTransformManager();

    Entity my_camera = utils::EntityManager::get().create();
    Camera* camera = mEngine->createCamera(my_camera);
    View* view = mEngine->createView();
    Scene* scene = mEngine->createScene();

    view->setCamera(camera);    


    uint32_t w, h;
    SDL_GL_GetDrawableSize(mWindow, (int*) &w, (int*) &h);

    camera->setProjection(45.0, double(w) / h, 0.1, 50, Camera::Fov::VERTICAL);
    view->setViewport({ 0, 0, w, h });

//    Viewport const& mViewport = {0, 0, w, h};
//    view->setViewport(mViewport);
    view->setPostProcessingEnabled(false);
    //
    view->setScene(scene);

    Skybox* skybox;

//    Scene* mScene = nullptr;
//    std::unique_ptr<IBL> mIBL;
//    Texture* mDirt = nullptr;

//    VertexBuffer* vertex_buffer = nullptr;
//    IndexBuffer* index_buffer = nullptr;
//    utils::Entity renderable;
//    const Material* material = nullptr;
//    MaterialInstance* material_instance = nullptr;


//    Engine *engine = Engine::create();
//    mBackend = mEngine->getBackend();

//    mEngine->destroy(&mEngine);
    skybox = Skybox::Builder().color({0.1, 0.125, 0.25, 1.0}).build(*mEngine);

//    /* create material */
//    if (nullptr == material_instance) {
//
//        std::string shader = R"SHADER(
//          void material(inout MaterialInputs material) {
//             prepareMaterial(material);
//             material.baseColor.rgb = float3(1.0, 1.0, 1.0);
//             material.metallic = 1.0;
//             material.roughness = 0.0;
//          }
//        )SHADER";
//
//        filamat::MaterialBuilder::init();
//        filamat::MaterialBuilder mat_builder = filamat::MaterialBuilder();
//        mat_builder.name("test");
//        mat_builder.material(shader.c_str());
//        mat_builder.doubleSided(true);
//        mat_builder.shading(filament::Shading::LIT);
//        mat_builder.targetApi(filamat::MaterialBuilder::TargetApi::ALL);
//
//        // this is added by JIAN
//        utils::JobSystem* jobsystem;
//        jobsystem = &mEngine->getJobSystem();
//        filamat::Package pkg = mat_builder.build(*jobsystem);
//        //filamat::Package pkg = mat_builder.build();
//        filament::Material::Builder fil_mat_builder = filament::Material::Builder();
//        fil_mat_builder.package(pkg.getData(), pkg.getSize());
//
//        material = fil_mat_builder.build(*mEngine);
//        material_instance = material->createInstance();
//    }


//    SDL_Init( SDL_INIT_EVERYTHING );
//    initSDL();
    
//    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
//    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
//    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 3 );
//    SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8 );
    
//    SDL_Window *window = SDL_CreateWindow("JIAN OpenGL", 0, 0, WIDTH, HEIGHT, SDL_WINDOW_OPENGL );




    scene->setSkybox(skybox);

//    if (mWindow) {
//        float dpiScaleX = 1.0f;
//        float dpiScaleY = 1.0f;
//
//        uint32_t w, h;
//        SDL_GL_GetDrawableSize(mWindow, (int*) &w, (int*) &h);
//        size_t mWidth = (size_t) w;
//        size_t mHeight = (size_t) h;
//
//        int virtualWidth, virtualHeight;
//        SDL_GetWindowSize(mWindow, &virtualWidth, &virtualHeight);
//        dpiScaleX = (float) w / virtualWidth;
//        dpiScaleY = (float) h / virtualHeight;
//
//        const uint32_t ww = mWidth;
//        const uint32_t hh = mHeight;
//
//        const float3 at(0, 0, -4);
//        const double ratio = double(hh) / double(ww);
//        const int sidebar = 0 * dpiScaleX;
//
//        const uint32_t mainWidth = std::max(1, (int) ww - sidebar);
//
//        double near = 0.1;
//        double far = 100;
//        camera->setLensProjection(28.0, double(mainWidth) / hh, near, far);
//
//        Viewport const& mViewport = {sidebar, 0, mainWidth, hh};
//        view->setViewport(mViewport);
//    }




    VertexBuffer* vb = VertexBuffer::Builder()
        .vertexCount(3)
        .bufferCount(1)
        .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT2, 0, 12)
        .attribute(VertexAttribute::COLOR, 0, VertexBuffer::AttributeType::UBYTE4, 8, 12)
        .normalized(VertexAttribute::COLOR)
        .build(*mEngine);
    vb->setBufferAt(*mEngine, 0,
        VertexBuffer::BufferDescriptor(TRIANGLE_VERTICES, 36, nullptr));
    IndexBuffer* ib = IndexBuffer::Builder()
        .indexCount(3)
        .bufferType(IndexBuffer::IndexType::USHORT)
        .build(*mEngine);
    ib->setBuffer(*mEngine,
                IndexBuffer::BufferDescriptor(TRIANGLE_INDICES, 6, nullptr));    


//     VertexBuffer* vertexBuffer =
//             VertexBuffer::Builder()
//                     .vertexCount(4)
//                     .bufferCount(2)
//                     .attribute(VertexAttribute::POSITION, 0,
//                             VertexBuffer::AttributeType::FLOAT3)
//                     .attribute(VertexAttribute::TANGENTS, 1,
//                             VertexBuffer::AttributeType::SHORT4)
//                     .normalized(VertexAttribute::TANGENTS)
//                     .build(*mEngine);
// 
//     vertexBuffer->setBufferAt(
//             *mEngine, 0,
//             VertexBuffer::BufferDescriptor(vertices, vertexBuffer->getVertexCount() * sizeof(vertices[0])));
//     vertexBuffer->setBufferAt(
//             *mEngine, 1,
//             VertexBuffer::BufferDescriptor(normals, vertexBuffer->getVertexCount() * sizeof(normals[0])));
// 
// 
//     IndexBuffer* indexBuffer =
//             IndexBuffer::Builder().indexCount(6).build(*mEngine);
// 
//     indexBuffer->setBuffer(
//             *mEngine, IndexBuffer::BufferDescriptor(indices, indexBuffer->getIndexCount() * sizeof(uint32_t)));


//    readMaterial(mEngine);

//    g_material->setDefaultParameter("baseColor", RgbType::LINEAR, float3{ 0.8, 0, 0 });
//    g_materialInstance = g_material->createInstance();

    
    Material* material = Material::Builder()
                                 .package((void*) BAKED_MATERIAL_PACKAGE,
                                         sizeof(BAKED_MATERIAL_PACKAGE))
                                 .build(*mEngine);
//    material->setDefaultParameter("baseColor", RgbType::LINEAR, float3{ 0.0, 1.0, 0 });
//    material->setDefaultParameter("metallic", 0.0f);
//    material->setDefaultParameter("roughness", 0.4f);
//    material->setDefaultParameter("reflectance", 0.5f);


    MaterialInstance* materialInstance = material->createInstance();

    Entity renderable = EntityManager::get().create();

    Entity light = EntityManager::get().create();


    LightManager::Builder(LightManager::Type::SUN)
            .color(Color::toLinear<ACCURATE>(sRGBColor(0.98f, 0.92f, 0.89f)))
            .intensity(500000)
            .direction({ 0.7, -1, -0.8 })
            .sunAngularRadius(1.9f)
            .castShadows(true)
            .build(*mEngine, light);

    scene->addEntity(light);

//    // build a quad
//    RenderableManager::Builder(1)
//            .boundingBox({ { -1, -1, -1 }, { 1, 1, 1 } })
//            .material(0, materialInstance)
//            .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, vertexBuffer,
//                    indexBuffer, 0, 6)
//            .culling(false)
//            .build(*mEngine, renderable);

    RenderableManager::Builder(1)
            .boundingBox({{ -1, -1, -1 }, { 1, 1, 1 }})
            .material(0, materialInstance)
            .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, vb, ib, 0, 3)
            .culling(false)
            .receiveShadows(false)
            .castShadows(false)
            .build(*mEngine, renderable);

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
        if (mRenderer->beginFrame(mSwapChain)) {
            // for each View
            mRenderer->render(view);
            mRenderer->endFrame();
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

    mEngine->destroy(my_camera);
    mEngine->destroy(skybox);

    return 0;


//    SDL_GLContext context = SDL_GL_CreateContext( mWindow );
//    
//    SDL_Event windowEvent;
//    
//    while ( true )
//    {
//        if ( SDL_PollEvent( &windowEvent ) )
//        {
//            if ( SDL_QUIT == windowEvent.type )
//            {
//                break;
//            }
//        }
//        
//        //draw OpenGL
//        SDL_GL_SwapWindow( mWindow );
//    }
//    SDL_GL_DeleteContext( context );
//    SDL_DestroyWindow( mWindow );
//    SDL_Quit();
//    
//
////    mEngine->destroy(&mEngine);
////    mEngine->destroy(mRenderer);
////    mEngine->destroy(mSwapChain);
////    mEngine->destroy(material);
//
//    return EXIT_SUCCESS;
}


