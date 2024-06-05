#pragma once

#include <engine/interface/filesystem/VirtualFileSystemInterface.h>
#include <engine/interface/render/RenderInterface.h>
#include <engine/interface/ResourceContainer.h>
#include <logger/logger.h>
#include <memory>

namespace engine
{
	namespace render
	{
		class CRenderInterface : public IRenderInterface
		{
		public:
			CRenderInterface(filesystem::IVirtualFileSystemInterface* vfs);
			~CRenderInterface() override;

			void Create(const FRenderInterfaceCreateInfo& render_interface_ci) override;
			void PostLoad() override;
			void Render(f32 dt) override;

			void UpdateSurfaceSize(i32 width, i32 height) override;

			const std::unique_ptr<FRenderDataPacket>& GetRenderPacket() override;
			const std::unique_ptr<IRenderGraph>& GetRenderGraph() override;
			const std::unique_ptr<IRenderWorld>& GetRenderWorld() override;
			const std::unique_ptr<IRenderContext>& GetRenderContext() override;
			const std::unique_ptr<IRenderResourceManager>& GetResourceManager() override;
		private:
			filesystem::IVirtualFileSystemInterface* m_pVFS{ nullptr };

			u32 m_frameCounter{ 0u };

			std::unique_ptr<FRenderDataPacket> m_pRenderPacket{};
			std::unique_ptr<IRenderGraph> m_pRenderGraph{};
			std::unique_ptr<IRenderWorld> m_pRenderWorld{};
			std::unique_ptr<IRenderContext> m_pRenderContext{};
			std::unique_ptr<IRenderResourceManager> m_pResourceManager{};
		};
	}
}