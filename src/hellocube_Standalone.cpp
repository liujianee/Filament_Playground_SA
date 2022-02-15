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

#include <filament/FilamentAPI.h>
#include <math/norm.h>
#include <math/mat3.h>

#include <cmath>
//#include "generated/resources/resources.h"

#define WIDTH 1024
#define HEIGHT 640

#define RELATIVE_ASSET_PATH ".."

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

const utils::Path& getRootAssetsPath() {
    static const utils::Path root = utils::Path::getCurrentExecutable().getParent() + RELATIVE_ASSET_PATH;
    return root;
}

static const char* IBL_FOLDER = "default_env";

/* Stored as Vertex(pos, norm, uv) */
int nfloats_cube = 288;
int nvertices_cube = 36;
int nindices_cube = 36;
float vertices_cube[] = {-1.000,-1.000,1.000,-1.000,0.000,0.000,0.375,0.000,-1.000,1.000,1.000,-1.000,0.000,0.000,0.625,0.000,-1.000,1.000,-1.000,-1.000,0.000,0.000,0.625,0.250,-1.000,-1.000,1.000,-1.000,0.000,0.000,0.375,0.000,-1.000,1.000,-1.000,-1.000,0.000,0.000,0.625,0.250,-1.000,-1.000,-1.000,-1.000,0.000,0.000,0.375,0.250,-1.000,-1.000,-1.000,0.000,-0.000,-1.000,0.375,0.250,-1.000,1.000,-1.000,0.000,-0.000,-1.000,0.625,0.250,1.000,1.000,-1.000,0.000,-0.000,-1.000,0.625,0.500,-1.000,-1.000,-1.000,0.000,-0.000,-1.000,0.375,0.250,1.000,1.000,-1.000,0.000,-0.000,-1.000,0.625,0.500,1.000,-1.000,-1.000,0.000,-0.000,-1.000,0.375,0.500,1.000,-1.000,-1.000,1.000,0.000,0.000,0.375,0.500,1.000,1.000,-1.000,1.000,0.000,0.000,0.625,0.500,1.000,1.000,1.000,1.000,0.000,0.000,0.625,0.750,1.000,-1.000,-1.000,1.000,0.000,0.000,0.375,0.500,1.000,1.000,1.000,1.000,0.000,0.000,0.625,0.750,1.000,-1.000,1.000,1.000,0.000,0.000,0.375,0.750,1.000,-1.000,1.000,0.000,0.000,1.000,0.375,0.750,1.000,1.000,1.000,0.000,0.000,1.000,0.625,0.750,-1.000,1.000,1.000,0.000,0.000,1.000,0.625,1.000,1.000,-1.000,1.000,0.000,0.000,1.000,0.375,0.750,-1.000,1.000,1.000,0.000,0.000,1.000,0.625,1.000,-1.000,-1.000,1.000,0.000,0.000,1.000,0.375,1.000,-1.000,-1.000,-1.000,0.000,-1.000,0.000,0.125,0.500,1.000,-1.000,-1.000,0.000,-1.000,0.000,0.375,0.500,1.000,-1.000,1.000,0.000,-1.000,0.000,0.375,0.750,-1.000,-1.000,-1.000,0.000,-1.000,0.000,0.125,0.500,1.000,-1.000,1.000,0.000,-1.000,0.000,0.375,0.750,-1.000,-1.000,1.000,0.000,-1.000,0.000,0.125,0.750,1.000,1.000,-1.000,0.000,1.000,-0.000,0.625,0.500,-1.000,1.000,-1.000,0.000,1.000,-0.000,0.875,0.500,-1.000,1.000,1.000,0.000,1.000,-0.000,0.875,0.750,1.000,1.000,-1.000,0.000,1.000,-0.000,0.625,0.500,-1.000,1.000,1.000,0.000,1.000,-0.000,0.875,0.750,1.000,1.000,1.000,0.000,1.000,-0.000,0.625,0.750};
uint32_t indices_cube[] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35};


/* -------------------------------------------------------------- */
class TangentsCube {
public:
  TangentsCube() = default;
  ~TangentsCube();
  int init(filament::Engine* engine);
  int shutdown();

public:
  std::vector<math::float3> positions;
  std::vector<math::float3> normals;
  std::vector<math::float2> uvs;
  std::vector<math::float4> tangents;
  std::vector<math::quatf> qtangents;
  std::vector<float> buffer_data;
  filament::VertexBuffer* vertex_buffer = nullptr;
  filament::IndexBuffer* index_buffer = nullptr;
  utils::Entity renderable;
  const filament::Material* material = nullptr;
  filament::MaterialInstance* material_instance = nullptr;
};

TangentsCube::~TangentsCube() {
  shutdown();
}

/*
   Stored as Vertex(pos, norm, uv)
   pos: offset 0,
   norm: offset 12
   uv: offset 24
   vertex stride: 8 * 4 = 32 bytes
*/
int TangentsCube::init(filament::Engine* engine) {
  
  /* de-interleave the vertices. */
  float* vert = vertices_cube;
  for (int i = 0; i < nvertices_cube; ++i) {
    float* pos = vert;
    float* norm = vert + 3;
    float* uv = vert + 6;
    positions.push_back(math::float3(pos[0], pos[1], pos[2]));
    normals.push_back(math::float3(norm[0], norm[1], norm[2]));
    uvs.push_back(math::float2(uv[0], uv[1]));
    vert += 8;
  }

  /* create the surface orientation builder and fill it up. */
  geometry::SurfaceOrientation::Builder builder = geometry::SurfaceOrientation::Builder();
  builder.normals(normals.data(), 0);
  builder.uvs(uvs.data(), 0);
  builder.positions(positions.data(), 0);
  builder.triangles((const filament::math::uint3*)indices_cube);
  builder.triangleCount(nindices_cube / 3);
  builder.vertexCount(nvertices_cube);

  /* calculate the qtangents. */
  geometry::SurfaceOrientation* sor = builder.build();
  qtangents.resize(nvertices_cube);
  sor->getQuats(qtangents.data(), qtangents.size(), 0);

  /* print the qtangents (for debugging purposes). */
  for (size_t i = 0; i < qtangents.size(); ++i) {
    math::quatf& q = qtangents[i];
    printf("fila.q: (%2.2f, %2.2f, %2.2f, %2.2f\n", q.x, q.y, q.z, q.w);
  }

  /* create vertex buffer. */
  //std::vector<float>* tmp = new std::vector<float>();
  for (int i = 0; i < nvertices_cube; ++i) {
    buffer_data.push_back(positions[i].x);
    buffer_data.push_back(positions[i].y);
    buffer_data.push_back(positions[i].z);
    buffer_data.push_back(qtangents[i].x);
    buffer_data.push_back(qtangents[i].y);
    buffer_data.push_back(qtangents[i].z);
    buffer_data.push_back(qtangents[i].w);
    buffer_data.push_back(uvs[i].x);
    buffer_data.push_back(uvs[i].y);
  }

  filament::VertexBuffer::Builder vbuf_builder = filament::VertexBuffer::Builder();
  vbuf_builder.vertexCount(nvertices_cube);
  vbuf_builder.bufferCount(1);
  vbuf_builder.attribute(filament::VertexAttribute::POSITION, 0, filament::VertexBuffer::AttributeType::FLOAT3, 0, 36);
  vbuf_builder.attribute(filament::VertexAttribute::TANGENTS, 0, filament::VertexBuffer::AttributeType::FLOAT4, 12, 36);
  vbuf_builder.attribute(filament::VertexAttribute::UV0, 0, filament::VertexBuffer::AttributeType::FLOAT2, 28, 36);

  vertex_buffer = vbuf_builder.build(*engine);
  vertex_buffer->setBufferAt(*engine, 0, filament::VertexBuffer::BufferDescriptor(buffer_data.data(), buffer_data.size() * sizeof(float), nullptr));

  /* create index buffer */
  filament::IndexBuffer::Builder ibuf_builder = filament::IndexBuffer::Builder();
  ibuf_builder.indexCount(nindices_cube);
  ibuf_builder.bufferType(filament::IndexBuffer::IndexType::UINT);

  index_buffer = ibuf_builder.build(*engine);
  index_buffer->setBuffer(*engine, filament::IndexBuffer::BufferDescriptor(indices_cube, nindices_cube * sizeof(uint32_t), nullptr));

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
    mat_builder.name("test");
    mat_builder.material(shader.c_str());
    mat_builder.doubleSided(true);
    mat_builder.shading(filament::Shading::LIT);
    mat_builder.targetApi(filamat::MaterialBuilder::TargetApi::ALL);


    // this is new
    utils::JobSystem* jobsystem;
    jobsystem = &engine->getJobSystem();
    filamat::Package pkg = mat_builder.build(*jobsystem);
    //filamat::Package pkg = mat_builder.build();
    filament::Material::Builder fil_mat_builder = filament::Material::Builder();
    fil_mat_builder.package(pkg.getData(), pkg.getSize());

    material = fil_mat_builder.build(*engine);
    material_instance = material->createInstance();
  }

  /* create entity */
  filament::RenderableManager::Builder ren_builder = filament::RenderableManager::Builder(1);
  renderable = utils::EntityManager::get().create();
  ren_builder.geometry(0, filament::RenderableManager::PrimitiveType::TRIANGLES, vertex_buffer, index_buffer, 0, nindices_cube);
  ren_builder.material(0, material_instance);
  ren_builder.culling(false);
  ren_builder.castShadows(false);
  ren_builder.receiveShadows(false);
  ren_builder.build(*engine, renderable);
    
  return 0;
}

int TangentsCube::shutdown() {
  int r = 0;
  return r;
}

/* -------------------------------------------------------------- */




int main()
{
    TangentsCube myCube;

//    std::string iblDirectory = getRootAssetsPath() + IBL_FOLDER;
    std::string iblDirectory = "../resource/default_env/";

//    const static uint32_t indices[] = { 0, 1, 2, 2, 3, 0 };
//
//    float3 vertices[] = {
//        { -10, 0, -10 },
//        { -10, 0, 10 },
//        { 10, 0, 10 },
//        { 10, 0, -10 },
//    };
//    
//    uint32_t colors[] = {
//        0xffff0000u,
//        0xff00ff00u,
//        0xff0000ffu,
//        0xff00ff00u,
//    };
//
//    short4 tbn = math::packSnorm16(
//            mat3f::packTangentFrame(math::mat3f{ float3{ 1.0f, 0.0f, 0.0f },
//                                            float3{ 0.0f, 0.0f, 1.0f },
//                                            float3{ 0.0f, 1.0f, 0.0f } })
//                    .xyzw);
//
//    const static math::short4 normals[]{ tbn, tbn, tbn, tbn };

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
    auto& rcm = engine->getRenderableManager();

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
    
    Skybox* skybox = Skybox::Builder().color({0.1, 0.125, 0.25, 1.0}).build(*engine);
//    Skybox* skybox = Skybox::Builder().color({0.0, 1.0, 0.0, 1.0}).build(*engine);
//    scene->setSkybox(skybox);
    view->setPostProcessingEnabled(false);
    
    view->setVisibleLayers(0x4, 0x4);

//     VertexBuffer* vertexBuffer =
//             VertexBuffer::Builder()
//                     .vertexCount(4)
//                     .bufferCount(2)
//                     .attribute(VertexAttribute::POSITION, 0,
//                             VertexBuffer::AttributeType::FLOAT3)
//                     .attribute(VertexAttribute::TANGENTS, 1,
//                             VertexBuffer::AttributeType::SHORT4)
//                     .normalized(VertexAttribute::TANGENTS)
// //                    .attribute(VertexAttribute::COLOR, 1,
// //                            VertexBuffer::AttributeType::UBYTE4)
// //                    .normalized(VertexAttribute::COLOR)
//                     .build(*engine);
// 
//     vertexBuffer->setBufferAt(
//             *engine, 0,
//             VertexBuffer::BufferDescriptor(vertices, vertexBuffer->getVertexCount() * sizeof(vertices[0])));
//     vertexBuffer->setBufferAt(
//             *engine, 1,
//             VertexBuffer::BufferDescriptor(normals, vertexBuffer->getVertexCount() * sizeof(normals[0])));
// //    vertexBuffer->setBufferAt(
// //            *engine, 1,
// //            VertexBuffer::BufferDescriptor(colors, vertexBuffer->getVertexCount() * sizeof(colors[0])));
// 
//     IndexBuffer* indexBuffer =
//             IndexBuffer::Builder().indexCount(6).build(*engine);
// 
//     indexBuffer->setBuffer(
//             *engine, IndexBuffer::BufferDescriptor(indices, indexBuffer->getIndexCount() * sizeof(uint32_t)));
// 
// 
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
// 
//     // build a quad
//     RenderableManager::Builder(1)
//             .boundingBox({ { -1, -1, -1 }, { 1, 1, 1 } })
//             .material(0, materialInstance)
//             .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, vertexBuffer,
//                     indexBuffer, 0, 6)
//             .culling(false)
//             .build(*engine, renderable);
// 
// 
//     scene->addEntity(renderable);
    mat4f origTransform = math::mat4f::translation(float3{ 0, 0, 40.0 }) * mat4f::rotation(0.2 * M_PI, float3{ 1, 1, 0 });
//     mat4 camTrans = camera->getModelMatrix();
//     tcm.setTransform(tcm.getInstance(renderable), origTransform);

    myCube.init(engine);
    scene->addEntity(myCube.renderable);
    auto cube_ti = tcm.getInstance(myCube.renderable);
    rcm.setCastShadows(rcm.getInstance(myCube.renderable), false);
    tcm.setTransform(cube_ti, origTransform);


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
        // float3 transform = float3(0,2,-4)
        mat4f transform = mat4f::translation(float3{ 0, -1, -50 }) * mat4f::rotation(g_discoAngle, float3{ 0, 0, 1 });
        transform *= origTransform;
        tcm.setTransform(cube_ti, transform);
        g_discoAngle += g_discoAngularSpeed * dT;

        lastTime = now;

        // Below is to animate one vertex, comment out if not needed:
        auto vb = myCube.vertex_buffer;
        void* verts = malloc(36*36);
        myCube.buffer_data[2] = 2 * sin(now * 4);
        memcpy(verts, myCube.buffer_data.data(), 36*36);
        vb->setBufferAt(*engine, 0, VertexBuffer::BufferDescriptor(verts, myCube.buffer_data.size() * sizeof(float), nullptr));
        // myCube.vertex_buffer->setBufferAt(*engine, 0, VertexBuffer::BufferDescriptor(myCube.buffer_data.data(), myCube.buffer_data.size() * sizeof(float), nullptr));
    }

    engine->destroy(my_camera);
//    engine->destroy(skybox);

    return 0;
}








