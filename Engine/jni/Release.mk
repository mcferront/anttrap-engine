
LOCAL_PATH := $(call my-dir)/..

include $(CLEAR_VARS)
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../Tools/3rdParty/Ogg/libvorbis/1.3.1/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../Tools/3rdParty/Ogg/libogg/1.2.0/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../Tools/3rdParty/OpenAL/1.1/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../Tools/3rdParty/Lua/5.1.3/src
LOCAL_C_INCLUDES += $(LOCAL_PATH)/OpenGL
LOCAL_C_INCLUDES += $(LOCAL_PATH)/jni
LOCAL_C_INCLUDES += $(LOCAL_PATH)

LOCAL_CFLAGS := -DLIB \
                -DANDROID \
                -D_RELEASE \
                -frtti
                
LOCAL_MODULE := Engine

LOCAL_SRC_FILES := \
   AllocSig.cpp \
   AnimAsset.cpp \
   AnimatedObject.cpp \
   AnimatedTexture.cpp \
   Animation2dComponent.cpp \
   Animation2dComposite.cpp \
   Animation3d.cpp \
   Animation3dComponent.cpp \
   Animation3dComposite.cpp \
   AnimationWorld.cpp \
   Asset.cpp \
   AudioWorld.cpp \
   Button.cpp \
   ButtonComponent.cpp \
   Camera.cpp \
   Channel.cpp \
   ChannelSystem.cpp \
   CollisionHandler.cpp \
   CollisionObject.cpp \
   Component.cpp \
   CompressedStreams.cpp \
   Data.cpp \
   Database.cpp \
   DataEntity.cpp \
   Debug.cpp \
   DebugGraphics.cpp \
   DefaultRasterizer.cpp \
   DefaultRenderer.cpp \
   EnginePch.cpp \
   Entity.cpp \
   FloatController.cpp \
   Font.cpp \
   FontMap.cpp \
   FrameMap.cpp \
   GenericMemoryAllocator.cpp \
   Geometry.cpp \
   HeightField.cpp \
   Hub.cpp \
   InputSystem.cpp \
   ISearchable.cpp \
   jni/AndroidEnv.cpp \
   jni/AndroidFileStreams.cpp \
   jni/AndroidMemoryAllocator.cpp \
   jni/AndroidThread.cpp \
   jni/AndroidVideoPlayer.cpp \
   jni/AndroidWindow.cpp \
   Line.cpp \
   Localization.cpp \
   Log.cpp \
   LookAtCameraController.cpp \
   LuaComponent.cpp \
   LuaEngineModule.cpp \
   LuaEventHandler.cpp \
   LuaGetProperty.cpp \
   LuaMethod.cpp \
   LuaObject.cpp \
   LuaScript.cpp \
   LuaSetProperty.cpp \
   LuaVM.cpp \
   MemoryAllocation.cpp \
   MemoryHeap.cpp \
   MemoryManager.cpp \
   MemoryStreams.cpp \
   MeshCollision.cpp \
   Model.cpp \
   ModelAsset.cpp \
   ModelComponent.cpp \
   OpenGL\GLMaterial.cpp \
   OpenGL\GLShader.cpp \
   OpenGL\GLVertexBuffer.cpp \
   Phantom.cpp \
   PhysicsObject.cpp \
   PhysicsWorld.cpp \
   PipeStreams.cpp \
   Proxy.cpp \
   Quad.cpp \
   Raycast.cpp \
   Registry.cpp \
   RegistryWorld.cpp \
   Renderer.cpp \
   RenderObject.cpp \
   RenderTree.cpp \
   RenderWorld.cpp \
   Resource.cpp \
   ResourceMaps.cpp \
   ResourcePreview.cpp \
   ResourceWorld.cpp \
   RigidBody.cpp \
   RingBuffer.cpp \
   SearchHierarchy.cpp \
   Serializer.cpp \
   Skeleton.cpp \
   Socket.cpp \
   Sprite.cpp \
   SpriteComponent.cpp \
   StringPool.cpp \
   TcpIpBurst.cpp \
   TcpIpPipe.cpp \
   TcpIpPipeListener.cpp \
   TcpIpStream.cpp \
   TextArea.cpp \
   TextAreaComponent.cpp \
   Texture.cpp \
   Threads.cpp \
   Timer.cpp \
   TouchComponent.cpp \
   TouchEvent.cpp \
   TouchObject.cpp \
   TouchWorld.cpp \
   Trigger.cpp \
   UtilityClock.cpp \
   UtilityMath.cpp \
   VectorController.cpp \
   VideoComponent.cpp \
   Viewport.cpp \
   WavAsset.cpp \
   Window.cpp 

include $(BUILD_STATIC_LIBRARY)
