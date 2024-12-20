#include "OGLContext.h"

#include "../../util/Logger.h"
#include "../../util/Profiler.h"
#include "../../runtime/Window.h"

#include "glad/glad.h"

#if defined(PLATFORM_WINDOWS)
#include "../../runtime/win64/WindowsNativeSurface.h"
#endif

#ifdef CreateSemaphore
#undef CreateSemaphore
#endif

namespace edge
{
	namespace gfx
	{
		namespace opengl
		{
			OpenGLContext::OpenGLContext(const ContextCreateInfo& create_info)
			{
				auto* window = create_info.window;
				auto* native_surface = window->GetNativeSurface();
#if defined(PLATFORM_WINDOWS)
				win32::WindowsNativeSurface* windows_surface = static_cast<win32::WindowsNativeSurface*>(native_surface);
				HDC hdc = GetDC(windows_surface->GetHandle());

				//// Set a pixel format for the device context
				//PIXELFORMATDESCRIPTOR pfd = { 0 };
				//pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
				//pfd.nVersion = 1;
				//pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
				//pfd.iPixelType = PFD_TYPE_RGBA;
				//pfd.cColorBits = 32;
				//pfd.cDepthBits = 24;
				//pfd.cStencilBits = 8;
				//pfd.iLayerType = PFD_MAIN_PLANE;
				//
				//int pixelFormat = ChoosePixelFormat(hdc, &pfd);
				//
				//// Create a temporary OpenGL context
				//HGLRC tempContext = wglCreateContext(hdc);
				//wglMakeCurrent(hdc, tempContext);
				//
				//if (!gladLoadGLLoader((GLADloadproc)wglGetProcAddress)) 
				//{
				//	EDGE_LOGE("Failed to initialize glad.");
				//}
				//
				//// Create a modern OpenGL context
				//int attribs[] = {
				//	WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
				//	WGL_CONTEXT_MINOR_VERSION_ARB, 3,
				//	WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
				//	0
				//};
				//
				//HGLRC modernContext = wglCreateContextAttribsARB(hdc, 0, attribs);
				//wglMakeCurrent(NULL, NULL);
				//wglDeleteContext(tempContext);
				//wglMakeCurrent(hdc, modernContext);
#endif

				assert(false && "NOT IMPLEMENTED");
			}

			OpenGLContext::~OpenGLContext()
			{

			}

			void OpenGLContext::Destroy(GFXResource* handle)
			{
				assert(false && "NOT IMPLEMENTED");
			}

			Fence* OpenGLContext::CreateFence()
			{
				assert(false && "NOT IMPLEMENTED");
				return nullptr;
			}

			Semaphore* OpenGLContext::CreateSemaphore()
			{
				assert(false && "NOT IMPLEMENTED");
				return nullptr;
			}

			const Queue* OpenGLContext::RequestQueue(QueueTypeFlags queue_type_flags, uint32_t queue_index)
			{
				assert(false && "NOT IMPLEMENTED");
				return nullptr;
			}

			Swapchain* OpenGLContext::CreateSwapchain(const SwapChainCreateInfo& create_info)
			{
				assert(false && "NOT IMPLEMENTED");
				return nullptr;
			}

			CommandPool* OpenGLContext::CreateCommandPool(const Queue* queue, CommandPoolResetMode reset_mode)
			{
				assert(false && "NOT IMPLEMENTED");
				return nullptr;
			}

			Buffer* OpenGLContext::CreateBuffer(const BufferCreateInfo& create_info)
			{
				assert(false && "NOT IMPLEMENTED");
				return nullptr;
			}

			Texture* OpenGLContext::CreateTexture(const TextureCreateInfo& create_info)
			{
				assert(false && "NOT IMPLEMENTED");
				return nullptr;
			}

			Texture* OpenGLContext::CreateSwapchainTexture(Swapchain* swapchain, uint32_t image_index)
			{
				assert(false && "NOT IMPLEMENTED");
				return nullptr;
			}

			Shader* OpenGLContext::CreateShader(const ShaderCreateInfo& create_info)
			{
				assert(false && "NOT IMPLEMENTED");
				return nullptr;
			}

			RootSignature* OpenGLContext::CreateRootSignature(const RootSignatureCreateInfo& create_info)
			{
				assert(false && "NOT IMPLEMENTED");
				return nullptr;
			}

			Pipeline* OpenGLContext::CreatePipeline(const GraphicsPipelineCreateInfo& create_info)
			{
				assert(false && "NOT IMPLEMENTED");
				return nullptr;
			}

			Pipeline* OpenGLContext::CreatePipeline(const ComputePipelineCreateInfo& create_info)
			{
				assert(false && "NOT IMPLEMENTED");
				return nullptr;
			}

			PipelineCache* OpenGLContext::CreatePipelineCache(const PipelineCacheCreateInfo& create_info)
			{
				assert(false && "NOT IMPLEMENTED");
				return nullptr;
			}

			DescriptorAllocator* OpenGLContext::CreateDescriptorAllocator()
			{
				assert(false && "NOT IMPLEMENTED");
				return nullptr;
			}

			SharedDescriptor* OpenGLContext::CreateSharedDescriptor(const SharedDescriptorCreateInfo& create_info)
			{
				assert(false && "NOT IMPLEMENTED");
				return nullptr;
			}

			Sampler* OpenGLContext::CreateSampler(const SamplerCreateInfo& create_info)
			{
				assert(false && "NOT IMPLEMENTED");
				return nullptr;
			}

			QueryPool* OpenGLContext::CreateQueryPool(const QueryPoolCreateInfo& create_info)
			{
				assert(false && "NOT IMPLEMENTED");
				return nullptr;
			}

			void OpenGLContext::WaitForFences(uint32_t fence_count, Fence** fences, uint64_t timeout)
			{
				assert(false && "NOT IMPLEMENTED");
			}

			void OpenGLContext::RequestMinimalBufferAlignment(BufferUsageFlags usage, uint64_t& alignment)
			{
				assert(false && "NOT IMPLEMENTED");
			}

			void OpenGLContext::RequestTimestampFrequency(float* frequency)
			{
				assert(false && "NOT IMPLEMENTED");
			}

			void OpenGLContext::RequestMemoryUsage(uint64_t* total_memory, uint64_t* used_memory)
			{
				assert(false && "NOT IMPLEMENTED");
			}
		}
	}
}