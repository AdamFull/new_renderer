#pragma once

#include <unordered_set>

#include <engine/interface/render/RenderInterface.h>
#include <engine/interface/EngineVariable.h>

#include "render_passes/BasePass.h"

namespace engine
{
	namespace render
	{
		class CRenderGraph : public IRenderGraph
		{
		public:
			CRenderGraph(IRenderInterface* render_interface);
			~CRenderGraph();

			void Create(const utl::vector<u8>& config, const utl::vector<u8>& schema) override;
			void PostLoad() override;

			void AddRenderResource(const std::string& name, const FRenderResource& resource_desc) override;
			void SetBackbufferResourceName(const std::string& name) override;

			const FRenderResource& GetRenderResource(const std::string& name) const override;
			const FRenderResource& GetRenderResource(FGraphHandle resource_id) const override;

			FGraphHandle GetRenderRersourceId(const std::string& name) const;

			u64 AddRenderPass(const std::string& name, const FRenderConfigRenderCreateInfo& render_pass_ci) override;
			u64 AddRenderPass(const std::string& name, const FRenderConfigComputeCreateInfo& compute_pass_ci) override;

			void AddStaticConditionNode(FRenderGraphStaticConditionNodeCreateInfo& create_info);
			void AddDynamicConditionNode(FRenderGraphDynamicConditionNodeCreateInfo& create_info);
			void AddGroupNode(FRenderGraphGroupNodeCreateInfo& create_info);
			void AddLayerNode(FRenderGraphGroupNodeCreateInfo& create_info);
			void AddRenderNode(FRenderGraphRenderPassNodeCreateInfo& create_info);
			void AddRenderNode(FRenderGraphComputePassNodeCreateInfo& create_info);

			FRenderGraphNodeHeader* CreateStaticConditionNode(FRenderGraphStaticConditionNodeCreateInfo& create_info);
			FRenderGraphNodeHeader* CreateDynamicConditionNode(FRenderGraphDynamicConditionNodeCreateInfo& create_info);
			FRenderGraphNodeHeader* CreateGroupNode(FRenderGraphGroupNodeCreateInfo& create_info);
			FRenderGraphNodeHeader* CreateRenderNode(FRenderGraphRenderPassNodeCreateInfo& create_info);
			FRenderGraphNodeHeader* CreateRenderNode(FRenderGraphComputePassNodeCreateInfo& create_info);

			void AddPassToLayer(const std::string& layer_name, std::unique_ptr<IBasePass>&& pass) override;

			void Build() override;
			void Update(f32 dt) override;
			void Execute() override;
		private:
			FTextureCreateInfo MakeTextureCI(EApiDataFormat format, ETextureCreateFlags flags);
			FTextureCreateInfo MakeTexture2DCI(EApiDataFormat format, ETextureCreateFlags flags);
			FTextureCreateInfo MakeTexture2DArrayCI(EApiDataFormat format, ETextureCreateFlags flags);
			FTextureCreateInfo MakeTexture3DCI(EApiDataFormat format, ETextureCreateFlags flags);
			FTextureCreateInfo MakeTexture3DArrayCI(EApiDataFormat format, ETextureCreateFlags flags);
			FTextureCreateInfo MakeTextureCubeCI(EApiDataFormat format, ETextureCreateFlags flags);
			FTextureCreateInfo MakeTextureCubeArrayCI(EApiDataFormat format, ETextureCreateFlags flags);

			void EvaluateRenderGraph(ERenderGraphEvaluateFlags flags);
			void EvaluateRenderGraphNode(ERenderGraphEvaluateFlags flags, FRenderGraphNodeHeader* node);
			void DestoyRenderGraph();
			void DestoyRenderGraphNode(FRenderGraphNodeHeader* node);
			void GraphUpdateBarriers(u64 pass_index);
			void ApplyRenderPassResources(u64 pass_index, const FRenderConfigPassCreateInfo* raw_pass_ci, FPassCreateInfo* pass_ci);
			void AapplySingleRenderResourceDependencies(FGraphHandle resource_id, ERenderResourceAccessFlagBits access_type, u64 pass_index, EQueueTypeFlagBits queue_type);
			void ApplyMultiRenderResourceDependencies(u64 pass_index, EQueueTypeFlagBits queue_type, const utl::hash_map<ERenderResourceGroupType, utl::vector<FRenderResourceBinding>>& associated_resources);

			bool WasGraphChanged();
		protected:
			utl::vector<std::unique_ptr<IBufferObject>> m_vBufferHandlers{};
			utl::vector<FRenderResource> m_vRenderResources{};
			utl::hash_map<std::string, FGraphHandle> m_nameToResource{};
			FGraphHandle m_backBufferResource{ INVALID_HANDLE };

			u64 m_executionHash{ 0ull };
			utl::vector<std::unique_ptr<IBasePass>> m_vRenderPasses{};
		private:
			FRenderGraphData m_graph;
			utl::hash_map<u64, u64> m_nameToLayer{};
			utl::vector<FRenderGraphNodeHeader*> m_vGraphNodes{};
			std::unique_ptr<IVariableBlock> m_pVariableBlock{};

			IRenderInterface* m_pRenderInterface{ nullptr };
		};
	}
}