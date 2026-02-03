// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "graphics_system.hpp"

#include "graphics_scene.hpp"

#include "base/log.hpp"
#include "base/utils.hpp"

#include "managers/resourcemanagerpool.hpp"

#include "rendering/r3d_messages.hpp"

namespace blunted {

  GraphicsSystem::GraphicsSystem() : systemType(e_SystemType_Graphics) {
    renderer3DTask = NULL;
    task = NULL;
  }

  GraphicsSystem::~GraphicsSystem() {
  }

  void GraphicsSystem::Initialize(const Properties &config) {

    textureResourceManager = boost::shared_ptr < ResourceManager<Texture> > (new ResourceManager<Texture>("texture"));
    vertexBufferResourceManager = boost::shared_ptr < ResourceManager<VertexBuffer> > (new ResourceManager<VertexBuffer>("vertexbuffer"));
    ResourceManagerPool::GetInstance().RegisterManager(e_ResourceType_Texture, textureResourceManager);
    ResourceManagerPool::GetInstance().RegisterManager(e_ResourceType_VertexBuffer, vertexBufferResourceManager);

    // create renderer (but don't start its thread — it will run on the main thread)
    if (config.Get("graphics3d_renderer", "opengl") == "opengl") renderer3DTask = new OpenGLRenderer3D();
    width = config.GetInt("context_x", 1280);
    height = config.GetInt("context_y", 720);
    bpp = config.GetInt("context_bpp", 32);
    bool fullscreen = config.GetBool("context_fullscreen", false);
    rendererRunningOnMainThread = false;

    // Queue up the CreateContext message — it will be the first thing processed
    // when the renderer loop starts on the main thread.
    boost::intrusive_ptr<Renderer3DMessage_CreateContext> createContext(new Renderer3DMessage_CreateContext(width, height, bpp, fullscreen));
    renderer3DTask->messageQueue.PushMessage(createContext);

    // Note: We do NOT wait for createContext here. The renderer isn't running yet.
    // It will be processed when StartRendererOnMainThread() is called.
    // Context creation success is checked there.

    task = new GraphicsTask(this);
    // Don't start GraphicsTask yet — it needs the renderer to be running first.
    // It will be started by StartRendererOnMainThread().
  }

  void GraphicsSystem::StartRendererOnMainThread() {
    // Start the GraphicsTask worker thread (orchestrates render phases)
    task->Run();

    // Run the renderer loop on the current (main) thread.
    // This blocks until shutdown. SDL/OpenGL require the main thread on macOS.
    rendererRunningOnMainThread = true;
    renderer3DTask->RunOnCurrentThread();
  }

  void GraphicsSystem::Exit() {
    // shutdown system task
    boost::intrusive_ptr<Message_Shutdown> shutdown(new Message_Shutdown());
    task->messageQueue.PushMessage(shutdown);
    shutdown->Wait();

    task->Join();
    delete task;
    task = NULL;

    textureResourceManager.reset();
    vertexBufferResourceManager.reset();

    // shutdown renderer — send shutdown message.
    // If running on main thread, RunOnCurrentThread() will return when it processes this.
    // No Join() needed since it wasn't spawned as a separate thread.
    boost::intrusive_ptr<Message_Shutdown> R3Dshutdown(new Message_Shutdown());
    renderer3DTask->messageQueue.PushMessage(R3Dshutdown);

    if (!rendererRunningOnMainThread) {
      R3Dshutdown->Wait();
      renderer3DTask->Join();
    }
    // Note: if rendererRunningOnMainThread, the renderer loop will exit naturally
    // when it processes the shutdown message. Cleanup happens after RunOnCurrentThread() returns.
    delete renderer3DTask;
    renderer3DTask = NULL;
  }

  e_SystemType GraphicsSystem::GetSystemType() const {
    return systemType;
  }

  ISystemScene *GraphicsSystem::CreateSystemScene(boost::shared_ptr<IScene> scene) {
    if (scene->GetSceneType() == e_SceneType_Scene2D) {
      GraphicsScene *graphicsScene = new GraphicsScene(this);
      scene->Attach(graphicsScene->GetInterpreter(e_SceneType_Scene2D));
      return graphicsScene;
    }
    if (scene->GetSceneType() == e_SceneType_Scene3D) {
      GraphicsScene *graphicsScene = new GraphicsScene(this);
      scene->Attach(graphicsScene->GetInterpreter(e_SceneType_Scene3D));
      return graphicsScene;
    }
    return NULL;
  }

  ISystemTask *GraphicsSystem::GetTask() {
    return task;
  }

  Renderer3D *GraphicsSystem::GetRenderer3D() {
    return renderer3DTask;
  }

  MessageQueue<Overlay2DQueueEntry> &GraphicsSystem::GetOverlay2DQueue() {
    return overlay2DQueue;
  }

}
