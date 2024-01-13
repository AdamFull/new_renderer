#pragma once

#include <unordered_set>

#include <engine/interface/render/RenderInterface.h>

#include "render_passes/BasePass.h"

namespace engine
{
	namespace render
	{
		class CRenderGraph : public IRenderGraph
		{
		public:
			void create();

			size_t addStaticResource(const std::string& name, const std::string& path);
			size_t addStaticResource(const std::string& name, const FTextureCreateInfo& texture_ci) override;
			size_t addRenderResource(const std::string& name, const FTextureCreateInfo& texture_ci) override;
			size_t addRenderResource(const std::string& name, const FBufferCreateInfo& buffer_ci) override;

			const FRenderResource& getRenderResource(const std::string& name) const override;
			const FRenderResource& getRenderResource(size_t resource_id) const override;

			size_t getRenderRersourceId(const std::string& name) const;

			size_t addRenderPass(const std::string& name, const FRenderConfigRenderCreateInfo& render_pass_ci) override;
			size_t addRenderPass(const std::string& name, const FRenderConfigComputeCreateInfo& compute_pass_ci) override;

			void bake() override;

			void execute(float dt) override;
		private:
			void applySingleRenderResourceDependencies(size_t resource_id, uint16_t access_type, size_t pass_index, uint32_t queue_type);
			void applyMultiRenderResourceDependencies(size_t pass_index, uint32_t queue_type, const std::unordered_map<uint32_t, std::vector<size_t>>& associated_resources);
		protected:
			std::vector<FRenderResource> m_vRenderResources{};
			std::unordered_map<std::string, size_t> m_nameToResource{};

			std::vector<std::unique_ptr<IBasePass>> m_vRenderPasses{};
			std::unordered_map<std::string, size_t> m_nameToPass{};
		private:
			IRenderInterface* m_pRenderInterface{ nullptr };
			IRenderContext* m_pRenderContext{ nullptr };
		};
	}
}