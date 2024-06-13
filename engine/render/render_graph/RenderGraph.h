#pragma once

#include <unordered_set>

#include <engine/interface/render/RenderInterface.h>
#include <engine/interface/EngineVariable.h>

#include <reactive/function.h>

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

			void Create(const utl::vector<u8>& config, const utl::vector<u8>& schema) override;
			void PostLoad() override;

			void AddRenderResource(const std::string& name, const FRenderResource& resource_desc) override;
			void SetBackbufferResourceName(const std::string& name) override;

			u64 AddSampler(const std::string& name, const FSamplerCreateInfo& sampler_ci);

			void PushDebugMarker(const std::string& name, u32 color, EQueueTypeFlagBits queue) override;
			void PopDebugMarker(const std::string& name, EQueueTypeFlagBits queue) override;

			void DestroyResource(u64 buffer_id);

			const FRenderResource& GetRenderResource(const std::string& name) const override;
			const FRenderResource& GetRenderResource(FGraphHandle resource_id) const override;

			FGraphHandle GetRenderRersourceId(const std::string& name) const;

			u64 AddRenderPass(const std::string& name, const FRenderConfigRenderCreateInfo& render_pass_ci) override;
			u64 AddRenderPass(const std::string& name, const FRenderConfigComputeCreateInfo& compute_pass_ci) override;

			void AddStaticConditionNode(FRenderGraphStaticConditionNodeCreateInfo& create_info);
			void AddDynamicConditionNode(FRenderGraphDynamicConditionNodeCreateInfo& create_info);
			void AddGroupNode(FRenderGraphGroupNodeCreateInfo& create_info);
			void AddRenderNode(FRenderGraphRenderPassNodeCreateInfo& create_info);
			void AddRenderNode(FRenderGraphComputePassNodeCreateInfo& create_info);

			FRenderGraphNodeHeader* CreateStaticConditionNode(FRenderGraphStaticConditionNodeCreateInfo& create_info);
			FRenderGraphNodeHeader* CreateDynamicConditionNode(FRenderGraphDynamicConditionNodeCreateInfo& create_info);
			FRenderGraphNodeHeader* CreateGroupNode(FRenderGraphGroupNodeCreateInfo& create_info);
			FRenderGraphNodeHeader* CreateRenderNode(FRenderGraphRenderPassNodeCreateInfo& create_info);
			FRenderGraphNodeHeader* CreateRenderNode(FRenderGraphComputePassNodeCreateInfo& create_info);

			void Build() override;
			void Update(f32 dt) override;
			void Execute(f32 dt) override;
		private:
			FTextureCreateInfo MakeTextureCI(EApiDataFormat format, ETextureCreateFlags flags);
			FTextureCreateInfo MakeTexture2DCI(EApiDataFormat format, ETextureCreateFlags flags);
			FTextureCreateInfo MakeTexture2DArrayCI(EApiDataFormat format, ETextureCreateFlags flags);
			FTextureCreateInfo MakeTexture3DCI(EApiDataFormat format, ETextureCreateFlags flags);
			FTextureCreateInfo MakeTexture3DArrayCI(EApiDataFormat format, ETextureCreateFlags flags);
			FTextureCreateInfo MakeTextureCubeCI(EApiDataFormat format, ETextureCreateFlags flags);
			FTextureCreateInfo MakeTextureCubeArrayCI(EApiDataFormat format, ETextureCreateFlags flags);

			void EvaluateRenderGraph(utl::vector<u64>& passes);
			void EvaluateRenderGraphNode(FRenderGraphNodeHeader* node, utl::vector<u64>& passes);
			void ApplyRenderPassResources(u64 pass_index, const FRenderConfigPassCreateInfo* raw_pass_ci, FPassCreateInfo* pass_ci);
			void AapplySingleRenderResourceDependencies(FGraphHandle resource_id, ERenderResourceAccessFlagBits access_type, u64 pass_index, EQueueTypeFlagBits queue_type);
			void ApplyMultiRenderResourceDependencies(u64 pass_index, EQueueTypeFlagBits queue_type, const utl::hash_map<ERenderResourceGroupType, utl::vector<FRenderResourceBinding>>& associated_resources);

			bool WasGraphChanged();
			void GenerateResourceBarriers(utl::hash_map<u64, FPassBuildInfo>& build_infos, bool generate_build_infos);
			void TraverseDependencies(u64 current_pass_index, u64 stack_count = 0ull);
			void TraverseDependencies2(u64 current_pass_index, u64 stack_count = 0ull);
			bool CheckPassDependencies(u64 dst_pass, u64 src_pass);
			void ReorderPasses(utl::vector<u64>& flattened_passes);
		protected:
			utl::vector<std::unique_ptr<IBufferObject>> m_vBufferHandlers{};
			utl::vector<FRenderResource> m_vRenderResources{};
			utl::hash_map<std::string, FGraphHandle> m_nameToResource{};
			FGraphHandle m_backBufferResource{ INVALID_HANDLE };

			u64 m_executionHash{ 0ull };
			utl::vector<std::unique_ptr<IBasePass>> m_vRenderPasses{};
			utl::hash_map<std::string, u64> m_nameToPass{};
		private:
			utl::vector<FRenderGraphNodeHeader*> m_vGraphNodes{};
			std::unique_ptr<IVariableBlock> m_pVariableBlock{};
			utl::hash_map<u64, utl::vector<FResourceBarrierInfo>> m_perPassResourceBarriers{};

			utl::vector<u64> m_vPassExecutionQueue{};
			utl::vector<std::unordered_set<u64>> m_vPassDependencies{};
			utl::vector<std::unordered_set<u64>> m_vPassMergeDependencies{};
			IRenderInterface* m_pRenderInterface{ nullptr };
		};
	}
}