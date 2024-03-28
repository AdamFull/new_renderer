#pragma once

#include <unordered_set>

#include <engine/interface/render/RenderInterface.h>

#include "render_passes/BasePass.h"

namespace engine
{
	namespace render
	{
		struct FDependencyTree
		{
			stl::vector<stl::unordered_set<u64>> passDependencies{};
		};

		class CRenderGraph : public IRenderGraph
		{
		public:
			CRenderGraph(IRenderInterface* render_interface);
			~CRenderGraph() = default;

			void create() override;

			i64 addViewport(const std::string& name, const FViewportCreateInfo& create_info) override;
			void setDefaultViewport(const std::string& name) override;
			const FViewportView& getViewport(const std::string& name) const override;
			const FViewportView& getViewport(u64 idx) const override;

			u64 addStaticResource(const stl::string& name, const stl::string& path) override;
			u64 addRenderResource(const stl::string& name, const FTextureCreateInfo& texture_ci) override;
			u64 addRenderResource(const stl::string& name, const FBufferCreateInfo& buffer_ci) override;
			void setBackbufferResourceName(const stl::string& name) override;

			void destroyResource(u64 buffer_id);

			const FRenderResource& getRenderResource(const stl::string& name) const override;
			const FRenderResource& getRenderResource(u64 resource_id) const override;

			u64 getRenderRersourceId(const stl::string& name) const;

			u64 addRenderPass(const stl::string& name, const FRenderConfigRenderCreateInfo& render_pass_ci) override;
			u64 addRenderPass(const stl::string& name, const FRenderConfigComputeCreateInfo& compute_pass_ci) override;

			void build() override;
			void update(f32 dt) override;
			void execute(f32 dt) override;
		private:
			void applyRenderPassResources(u64 pass_index, const FRenderConfigPassCreateInfo* raw_pass_ci, FPassCreateInfo* pass_ci);
			void applySingleRenderResourceDependencies(u64 resource_id, ERenderResourceAccessFlagBits access_type, u64 pass_index, EQueueTypeFlagBits queue_type);
			void applyMultiRenderResourceDependencies(u64 pass_index, EQueueTypeFlagBits queue_type, const stl::unordered_map<ERenderResourceGroupType, stl::vector<u64>>& associated_resources);

			void traverseDependencies(u64 current_pass_index, u64 stack_count = 0ull);
			void traverseDependencies2(u64 current_pass_index, u64 stack_count = 0ull);
			bool checkPassDependencies(u64 dst_pass, u64 src_pass);
			void reorderPasses(stl::vector<u64>& flattened_passes);
		protected:
			stl::vector<stl::unique_ptr<IBufferObject>> m_vBufferHandlers{};
			stl::vector<FRenderResource> m_vRenderResources{};
			stl::unordered_map<stl::string, u64> m_nameToResource{};
			u64 m_backBufferResource{ ~0ull };

			std::vector<FViewportView> m_vViewports{};
			stl::unordered_map<stl::string, u64> m_nameToViewport{};
			u64 m_defaultViewport{ ~0ull };

			stl::vector<stl::unique_ptr<IBasePass>> m_vRenderPasses{};
			stl::unordered_map<stl::string, u64> m_nameToPass{};
		private:
			stl::unordered_map<u64, stl::vector<FResourceBarrierInfo>> m_perPassResourceBarriers{};

			stl::vector<u64> m_vPassExecutionQueue{};
			stl::vector<u64> m_vPassOrder{};
			stl::vector<stl::unordered_set<u64>> m_vPassDependencies{};
			stl::vector<stl::unordered_set<u64>> m_vPassMergeDependencies{};
			IRenderInterface* m_pRenderInterface{ nullptr };
		};
	}
}