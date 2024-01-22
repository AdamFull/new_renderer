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
			std::vector<std::unordered_set<u64>> passDependencies{};
		};

		class CRenderGraph : public IRenderGraph
		{
		public:
			CRenderGraph(IRenderInterface* render_interface);
			~CRenderGraph() = default;

			void create() override;

			u64 addStaticResource(const std::string& name, const std::string& path) override;
			u64 addRenderResource(const std::string& name, const FTextureCreateInfo& texture_ci) override;
			u64 addRenderResource(const std::string& name, const FBufferCreateInfo& buffer_ci) override;

			const FRenderResource& getRenderResource(const std::string& name) const override;
			const FRenderResource& getRenderResource(u64 resource_id) const override;

			u64 getRenderRersourceId(const std::string& name) const;

			u64 addRenderPass(const std::string& name, const FRenderConfigRenderCreateInfo& render_pass_ci) override;
			u64 addRenderPass(const std::string& name, const FRenderConfigComputeCreateInfo& compute_pass_ci) override;

			void build() override;

			void execute(f32 dt) override;
		private:
			void applyRenderPassResources(u64 pass_index, const FRenderConfigPassCreateInfo* raw_pass_ci, FPassCreateInfo* pass_ci);
			void applySingleRenderResourceDependencies(u64 resource_id, ERenderResourceAccessFlagBits access_type, u64 pass_index, EQueueTypeFlagBits queue_type);
			void applyMultiRenderResourceDependencies(u64 pass_index, EQueueTypeFlagBits queue_type, const std::unordered_map<ERenderResourceGroupType, std::vector<u64>>& associated_resources);

			void traverseDependencies(u64 current_pass_index, u64 stack_count = 0ull);
			void traverseDependencies2(u64 current_pass_index, u64 stack_count = 0ull);
			bool checkPassDependencies(u64 dst_pass, u64 src_pass);
			void reorderPasses(std::vector<u64>& flattened_passes);
		protected:
			std::vector<std::unique_ptr<IBufferObject>> m_vBufferHandlers{};
			std::vector<FRenderResource> m_vRenderResources{};
			std::unordered_map<std::string, u64> m_nameToResource{};
			u64 m_backBufferResource{ ~0ull };

			std::vector<std::unique_ptr<IBasePass>> m_vRenderPasses{};
			std::unordered_map<std::string, u64> m_nameToPass{};
		private:
			std::unordered_map<u64, std::vector<FResourceBarrierInfo>> m_perPassResourceBarriers{};

			std::vector<u64> m_vPassExecutionQueue{};
			std::vector<u64> m_vPassOrder{};
			std::vector<std::unordered_set<u64>> m_vPassDependencies{};
			std::vector<std::unordered_set<u64>> m_vPassMergeDependencies{};
			IRenderInterface* m_pRenderInterface{ nullptr };
		};
	}
}