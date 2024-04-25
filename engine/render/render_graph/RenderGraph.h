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
			utl::vector<std::unordered_set<u64>> passDependencies{};
		};

		class CRenderGraph : public IRenderGraph
		{
		public:
			CRenderGraph(IRenderInterface* render_interface);
			~CRenderGraph() = default;

			void create() override;

			i64 addViewport(const std::string& name, const FViewportCreateInfo& create_info) override;
			void setDefaultViewport(const std::string& name) override;
			const FViewportView& getViewportView(const std::string& name) const override;
			const FViewportView& getViewportView(u64 idx) const override;
			const FViewportView& getDefaultViewportView() const override;
			const FViewport& getViewport(const std::string& name) const override;
			const FViewport& getViewport(u64 idx) const override;
			const FViewport& getDefaultViewport() const override;
			u64 getDefaultViewportIndex() const override;

			u64 addStaticResource(const std::string& name, const std::string& path) override;
			u64 addRenderResource(const std::string& name, const FTextureCreateInfo& texture_ci) override;
			u64 addRenderResource(const std::string& name, const FBufferCreateInfo& buffer_ci) override;
			void setBackbufferResourceName(const std::string& name) override;

			void pushDebugMarker(const std::string& name, u32 color, EQueueTypeFlagBits queue) override;
			void popDebugMarker(const std::string& name, EQueueTypeFlagBits queue) override;

			void destroyResource(u64 buffer_id);

			const FRenderResource& getRenderResource(const std::string& name) const override;
			const FRenderResource& getRenderResource(u64 resource_id) const override;

			u64 getRenderRersourceId(const std::string& name) const;

			u64 addRenderPass(const std::string& name, const FRenderConfigRenderCreateInfo& render_pass_ci) override;
			u64 addRenderPass(const std::string& name, const FRenderConfigComputeCreateInfo& compute_pass_ci) override;

			void build() override;
			void update(f32 dt) override;
			void execute(f32 dt) override;
		private:
			void applyRenderPassResources(u64 pass_index, const FRenderConfigPassCreateInfo* raw_pass_ci, FPassCreateInfo* pass_ci);
			void applySingleRenderResourceDependencies(u64 resource_id, ERenderResourceAccessFlagBits access_type, u64 pass_index, EQueueTypeFlagBits queue_type);
			void applyMultiRenderResourceDependencies(u64 pass_index, EQueueTypeFlagBits queue_type, const utl::hash_map<ERenderResourceGroupType, utl::vector<FRenderResourceBinding>>& associated_resources);

			bool wasGraphChanged();
			void generateResourceBarriers(utl::hash_map<u64, FPassBuildInfo>& build_infos, bool generate_build_infos);
			void traverseDependencies(u64 current_pass_index, u64 stack_count = 0ull);
			void traverseDependencies2(u64 current_pass_index, u64 stack_count = 0ull);
			bool checkPassDependencies(u64 dst_pass, u64 src_pass);
			void reorderPasses(utl::vector<u64>& flattened_passes);
		protected:
			utl::vector<std::unique_ptr<IBufferObject>> m_vBufferHandlers{};
			utl::vector<FRenderResource> m_vRenderResources{};
			utl::hash_map<std::string, u64> m_nameToResource{};
			u64 m_backBufferResource{ INVALID_HANDLE };

			utl::vector<FViewportView> m_vViewports{};
			utl::hash_map<std::string, u64> m_nameToViewport{};
			u64 m_defaultViewport{ INVALID_HANDLE };

			utl::vector<FViewport> m_vViewportBuffer{};

			u64 m_executionHash{ 0ull };
			utl::vector<std::unique_ptr<IBasePass>> m_vRenderPasses{};
			utl::hash_map<std::string, u64> m_nameToPass{};
		private:
			utl::hash_map<u64, utl::vector<FResourceBarrierInfo>> m_perPassResourceBarriers{};

			utl::vector<u64> m_vPassExecutionQueue{};
			utl::vector<std::unordered_set<u64>> m_vPassDependencies{};
			utl::vector<std::unordered_set<u64>> m_vPassMergeDependencies{};
			IRenderInterface* m_pRenderInterface{ nullptr };
		};
	}
}