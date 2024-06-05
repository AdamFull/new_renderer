#pragma once

#include <engine/interface/filesystem/VirtualFileSystemInterface.h>
#include <engine/interface/render/RenderInterface.h>
#include <engine/interface/ResourceContainer.h>
#include <memory>

namespace engine
{
	namespace render
	{
		using resource_container_t = CResourceContainer<IBufferObject, ITextureObject, IShaderObject, IVertexHandler>;

		class CRenderResourceManager : public IRenderResourceManager
		{
		public:
			CRenderResourceManager(IRenderInterface* render_interface, filesystem::IVirtualFileSystemInterface* vfs);
			~CRenderResourceManager() = default;

			void Create() override;

			u64 CreateTexture(const std::string& name, const FTextureCreateInfo& create_info) override;
			u64 LoadTexture(const std::string& path) override;
			const std::unique_ptr<ITextureObject>& GetTextureObject(u64 id) override;

			u64 CreateShader(const std::string& name, const FGraphicsShaderCreateInfo& create_info) override;
			u64 CreateShader(const std::string& name, const FComputeShaderCreateInfo& create_info) override;
			const std::unique_ptr<IShaderObject>& GetShaderObject(u64 id) override;

			u64 CreateBuffer(const std::string& name, const FBufferCreateInfo& create_info) override;
			const std::unique_ptr<IBufferObject>& GetBufferObject(u64 id) override;

			u64 CreateVertexHandler(const std::string& name, const FVertexHandlerCreateInfo& create_info) override;
			const std::unique_ptr<IVertexHandler>& GetVertexHandler(u64 id) override;

		protected:
			void LoadShaderStages(const std::string& name, utl::vector<FShaderCreationBlockInfo>& stages);
		private:
			std::unique_ptr<IResourceLoader> m_pResourceLoader;
			std::unique_ptr<resource_container_t> m_pResources;

			filesystem::IVirtualFileSystemInterface* m_pVFS{ nullptr };
			IRenderInterface* m_pRenderInterface{ nullptr };
		};
	}
}