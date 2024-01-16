#pragma once

#include <engine/interface/SharedStructure.h>
#include "BaseEnumerators.h"
#include "BufferStructures.h"
#include "LightStructures.h"
#include "MaterialStructures.h"
#include "RenderingStructures.h"
#include "ShaderStructures.h"
#include "TextureStructures.h"
#include "VertexStructures.h"
#include "RenderGraph.h"

#include <memory>
#include <typeindex>

namespace engine
{
	namespace render
	{
		class IRenderContext;
		class IRenderInterface;

#ifndef BUILD_WITHOUT_DEBUGGIND
		namespace debug_mapping
		{
			inline constexpr std::pair<size_t, const char*> uniform_to_name_mapping[] =
			{
				{ utl::fnv1a_64_hash("GlobalUniformFrameData"), "GlobalUniformFrameData" },
				{ utl::fnv1a_64_hash("UniformMeshData"), "UniformMeshData" },
				{ utl::fnv1a_64_hash("UniformSkinningJoints"), "UniformSkinningJoints" },
			};

			constexpr const char* find(size_t key) 
			{
				for (const auto& kv : uniform_to_name_mapping)
				{
					if (kv.first == key)
						return kv.second;
				}

				return nullptr;
			}
		}
#endif

		struct FGlobalUniformCreateInfo
		{
			std::unordered_map<size_t, size_t> uniforms{};
		};

		struct FGlobalUniformUpdateInfo
		{
			std::unordered_map<size_t, FUniformDataView> uniforms{};
		};

		struct FRenderCamera
		{
			float fov{ 45.f };
			float nearp{ 0.1f };
			float farp{ 512.f };

			glm::mat4 view{ 1.f };
			glm::mat4 invView{ 1.f };

			glm::mat4 projection{ 1.f };
			glm::mat4 invProjection{ 1.f };

			glm::mat4 viewProjection{ 1.f };
			glm::mat4 invViewProjection{ 1.f };

			glm::vec3 up{ 0.f };
			glm::vec3 forward{ 0.f };
			glm::vec3 right{ 0.f };

			glm::vec2 dimensions{};

			FFrustum frustum{};
		};

		struct FRenderNode
		{
			glm::mat4 model{};
		};

		class CUniformHandle
		{
		public:
			template<class _Ty>
			void create(const _Ty& block)
			{
				m_data.resize(block.size);
				m_uniforms = block.uniforms;
			}

			// For render script
			void set(const char* name, const void* raw_data, size_t size)
			{
				auto found = m_uniforms.find(utl::fnv1a_64_hash(name));
				if (found == m_uniforms.end() || !raw_data)
					return;

				auto& uniform = found->second;

				if (size == 0ull)
					size = uniform.size;

				std::memcpy(m_data.data() + uniform.offset, raw_data, size);
			}

			template<class _Ty>
			void set(const char* name, const _Ty& object, size_t size = 0ull)
			{
				auto found = m_uniforms.find(utl::fnv1a_64_hash(name));
				if (found == m_uniforms.end())
					return;

				auto& uniform = found->second;

				if (size == 0ull)
					size = (std::min)(sizeof(_Ty), uniform.size);

				std::memcpy(m_data.data() + uniform.offset, &object, size);
			}

			template<class _Ty>
			_Ty* get(const char* name)
			{
				auto found = m_uniforms.find(utl::fnv1a_64_hash(name));
				if (found == m_uniforms.end())
					return nullptr;

				auto& uniform = found->second;

				if (sizeof(_Ty) > uniform.size)
					return nullptr;

				return reinterpret_cast<_Ty*>(m_data.data() + uniform.offset);
			}

			const std::vector<char>& data() const
			{
				return m_data;
			}
		private:
			std::unordered_map<size_t, FReflectionUniform> m_uniforms{};
			std::vector<char> m_data{};
		};

		class IShaderObject
		{
		public:
			virtual ~IShaderObject() = default;

			virtual void create(const std::string& name, const FGraphicsShaderCreateInfo& create_info) = 0;
			virtual void create(const std::string& name, const FComputeShaderCreateInfo& create_info) = 0;

			virtual const std::unique_ptr<CUniformHandle>& getUniformBlock(const char* name) = 0;
			virtual const std::unique_ptr<CUniformHandle>& getConstantBlock(const char* name) = 0;

			virtual void bindBuffer(const char* name, size_t buffer_resource) = 0;
			virtual void bindTexture(const char* name, size_t texture_resource, uint32_t mip = 0u) = 0;

			virtual void flushDescriptors() = 0;
			virtual void flushConstants(EQueueTypeFlagBits queue) = 0;
			virtual void bind(EQueueTypeFlagBits queue) = 0;

			virtual size_t handle() const = 0;

			virtual const FShaderReflectionData& getReflection() const = 0;
		};


		class CGPUArrayHolder
		{
		public:
			template<class _Ty>
			const std::unique_ptr<IGPUArrayHandle>& registerResource(std::unique_ptr<IGPUArrayHandle>&& new_handle)
			{
				const std::type_index type_index(typeid(_Ty));
				return m_handles[type_index] = std::move(new_handle);
			}

			template<class _Ty>
			const std::unique_ptr<IGPUArrayHandle>& getResource()
			{
				const std::type_index type_index(typeid(_Ty));
				return m_handles[type_index];
			}

			void flush()
			{
				for (const auto& [id, handle] : m_handles)
					handle->flush();
			}
		private:
			std::unordered_map<std::type_index, std::unique_ptr<IGPUArrayHandle>> m_handles{};
		};

		struct FLightBuffer
		{
			std::unique_ptr<IGPUArrayHandle> lights{};
			std::unique_ptr<IGPUArrayHandle> shadows{};
		};

		struct FRenderSceneStats
		{
			size_t vboCount{ 0ull };
			size_t primitiveCount{ 0ull };
			size_t materialCount{ 0ull };
			size_t nodeCount{ 0ull };

			// Type is a hash of all used attributes
			std::unordered_map<size_t, size_t> perTypeVBOCount{};
		};

		struct FCullingInfo
		{
			size_t cullerID{};
			ERenderGeometryCullerType type{};
		};

		struct FGeometryBatchRange
		{
			size_t offset{ 0ull };
			size_t count{ 0ull };
		};

		struct FRenderDataPacket
		{
			FRenderDataPacket()
			{
				//resources = std::make_unique<CGPUArrayHolder>();
			}

			FRenderCamera camera{};
			FRenderSceneStats stats{};
		};

		struct FRenderWorldStats
		{
			size_t directionalLightCount{ 0ull };
			size_t pointLightCount{ 0ull };
			size_t spotLightCount{ 0ull };

			size_t cascadedShadowCount{ 0ull };
			size_t omniShadowCount{ 0ull };
			size_t spotShadowCount{ 0ull };

			size_t skinCount{ 0ull };

			size_t drawCommandCount{ 0ull };
		};

		class IRenderContext
		{
		public:
			virtual ~IRenderContext() = default;

			virtual void create(const FRenderContextCreateInfo& context_ci) = 0;
			virtual void createGlobalDescriptor(const FGlobalUniformCreateInfo& uniform_ci) = 0;
			virtual void updateGlobalDescriptorData(const FGlobalUniformUpdateInfo& data_view) = 0;

			virtual size_t createTexture(const std::string& name, const FTextureCreateInfo& texture_ci, FTextureUploadInfo&& upload_info) = 0;
			virtual size_t getTextureId(const std::string& name) = 0;
			virtual size_t createRenderTargets(const std::string& name, const FTextureCreateInfo& texture_ci) = 0;
			virtual const FTextureCreateInfo& getTextureInfo(size_t texture_id) = 0;
			virtual void destroyTexture(size_t resource_id) = 0;

			virtual size_t createShader(const std::string& name, const FGraphicsShaderCreateInfo& shader_ci, const  std::vector<FShaderCreationBlockInfo>& shader_stages) = 0;
			virtual size_t createShader(const std::string& name, const FComputeShaderCreateInfo& shader_ci, const  std::vector<FShaderCreationBlockInfo>& shader_stages) = 0;
			virtual FShaderReflectionData getShaderReflection(const size_t& shader_view) = 0;
			virtual void updateShaderResources(const FShaderResourceCommit& resources, EShaderResourcesUpdateFlags flags = EShaderResourcesUpdateFlagBits::eAll) = 0;
			virtual void flushConstantRanges(size_t shader_id, const std::unordered_map<size_t, FUniformDataView>& constants, EQueueTypeFlagBits queue_type) = 0;
			virtual void bindShader(size_t shader_id, EQueueTypeFlagBits queue_type) = 0;
			virtual void destroyShader(size_t resource_id) = 0;

			virtual size_t createBuffer(const std::string& name, const FBufferCreateInfo& buffer_ci) = 0;
			virtual void uploadDataToBuffer(const FBufferUploadInfo& upload_info) = 0;
			virtual FBufferView getBufferView(size_t buffer_id) = 0;
			virtual void bindVertexBuffers(const FVertexBufferBindingInfo& binding_info, EQueueTypeFlagBits queue) = 0;
			virtual void bindIndexBuffer(const FIndexBufferBindingInfo& binding_info, EQueueTypeFlagBits queue) = 0;
			virtual void mapBufferMemory(size_t buffer_id, void*& memory) = 0;
			virtual void unmapBufferMemory(size_t buffer_id) = 0;
			virtual void flushBuffer(size_t buffer_id, size_t size, size_t offset) = 0;
			virtual void destroyBuffer(size_t resource_id) = 0;

			virtual void pushG2CBarriers(EQueueTypeFlagBits queue_type, const std::vector<size_t>& textures) = 0;
			virtual void pushC2CBarrier(EQueueTypeFlagBits queue_type, const std::vector<size_t>& texture_ids) = 0;
			virtual void pushC2GBarrier(EQueueTypeFlagBits queue_type, const std::vector<size_t>& texture_ids) = 0;

			virtual void beginRendering(const FBeginRenderingInfo& rendering_info) = 0;
			virtual void endRendering(const FBeginRenderingInfo& rendering_info) = 0;

			virtual void beginRendering2(const FBeginRendering2Info& rendering_info) = 0;
			virtual void endRendering2(EQueueTypeFlagBits queue) = 0;

			virtual void beginCompute(const FBeginComputeInfo& rendering_info) = 0;
			virtual void endCompute(const FBeginComputeInfo& rendering_info) = 0;

			virtual void setViewport(float width, float height, bool flip_viewport) = 0;
			virtual void setScissorRect(int32_t offset_x, int32_t offset_y, uint32_t width, uint32_t height) = 0;

			virtual void draw(EQueueTypeFlagBits queue_type, const FGeometryDrawInfo& draw_info) = 0;
			virtual void draw(EQueueTypeFlagBits queue_type, const FIndirectGeometryDrawInfo& draw_info) = 0;
			virtual void dispatch(EQueueTypeFlagBits queue_type, const FDispatchInfo& dispatch_info) = 0;
			virtual void dispatch2(EQueueTypeFlagBits queue_type, const FDispatch2Info& dispatch_group) = 0;
			virtual void dispatch2(EQueueTypeFlagBits queue_type, const FDispatchIndirectInfo& dispatch_info) = 0;

			virtual size_t getTextureResource(const char* name) = 0;
			virtual size_t getShaderResource(const char* name) = 0;

			virtual FPoint2D<uint32_t> getRenderResolution() = 0;
			virtual FRect2D<float> getRenderRectFromRT(size_t texture_id) = 0;
			virtual FRect2D<float> getScreenRenderRect() = 0;

			virtual uint32_t getDepthFormat(bool depth_only) = 0;
			virtual uint32_t getApiFormat(EApiDataFormat global_format) = 0;
			virtual uint32_t getPresentFormat() = 0;

			virtual void updateSurface(int32_t surface_width, int32_t surface_height) = 0;

			virtual void beginFrameCommandBuffers() = 0;
			virtual void endFrameCommandBuffers() = 0;

			virtual bool isSwapChainFinished() = 0;

			virtual void beginDebugLabel(const std::string& label, EQueueTypeFlagBits queue_type, uint32_t color = 0xffffffff) = 0;
			virtual void insertDebugLabel(const std::string& label, EQueueTypeFlagBits queue_type, uint32_t color = 0xffffffff) = 0;
			virtual void endDebugLabel(EQueueTypeFlagBits queue_type) = 0;
		};

		class IRenderWorld
		{
		public:
			virtual ~IRenderWorld() = default;

			virtual void create() = 0;

			virtual size_t registerFrustum(ERenderGeometryCullerType type) = 0;
			virtual void updateFrustum(size_t frustum_id, const glm::mat4& view, const glm::mat4& projection) = 0;

			virtual void addDrawGeometry(const std::vector<FGeometryRawBatch>& new_geometry) = 0;

			virtual void render(float dt) = 0;

			virtual void addLight(const FDirectionalLightCommit& light) = 0;
			virtual void addLight(const FPointLightCommit& light) = 0;
			virtual void addLight(const FSpotLightCommit& light) = 0;

			virtual void addShadow(const FCascadedShadowCommit& commit) = 0;
			virtual void addShadow(const FOmniShadowCommit& commit) = 0;
			virtual void addShadow(const FSpotShadowCommit& commit) = 0;

			virtual void updateNode(uint32_t node_id, const FSceneNode& node) = 0;
			virtual void updateSkin(const std::array<glm::mat4, 128ull>& matrices) = 0;

			virtual void addMaterial(const FMaterial& material) = 0;
			virtual void addMaterials(const std::vector<FMaterial>& materials) = 0;

			virtual const std::vector<std::unique_ptr<IBufferObject>>& getDrawBuffers() = 0;

			virtual const std::unordered_map<ERenderGeometryCullerType, std::vector<FGeometryBatchRange>>& getAllGeometryRanges() const = 0;
			virtual const std::vector<FGeometryBatchRange>& getGeometryRanges(ERenderGeometryCullerType type) const = 0;
			virtual const FGeometryCPUBatch& getGeometryBatch(size_t index) const = 0;

			virtual FRenderWorldStats& getStats() = 0;
			virtual const FRenderWorldStats& getStats() const = 0;
		};

		class IRenderInterface
		{
		public:
			virtual ~IRenderInterface() = default;

			virtual void create(const FRenderInterfaceCreateInfo& render_interface_ci) = 0;
			virtual void render(float dt) = 0;

			virtual size_t createTexture(const std::string& path) = 0;
			virtual size_t loadTexture(const std::string& path, FTextureCreateInfo& create_info, FTextureUploadInfo& upload_info) = 0;

			virtual size_t createShader(const std::string& name_info, const FGraphicsShaderCreateInfo& create_info) = 0;
			virtual size_t createShader(const std::string& name_info, const FComputeShaderCreateInfo& create_info) = 0;

			virtual size_t createVertexHandler(const std::string& name, const FVertexHandlerCreateInfo& handler_ci) = 0;
			virtual const std::unique_ptr<IVertexHandler>& getVertexHandler(size_t id) = 0;

			virtual FShaderUploadInfo getShaderNameTag(const std::string& name) = 0;
			virtual void createRendering(const FRenderingStepCreateInfo& rendering_ci) = 0;

			virtual void updateSurfaceSize(int32_t width, int32_t height) = 0;

			virtual const std::unique_ptr<FRenderDataPacket>& getRenderPacket() = 0;
			virtual const std::unique_ptr<IRenderGraph>& getRenderGraph() = 0;
			virtual const std::unique_ptr<IRenderWorld>& getRenderWorld() = 0;
			virtual const std::unique_ptr<IRenderContext>& getRenderContext() = 0;
		};


		class IRenderGraphStep
		{
		public:
			virtual ~IRenderGraphStep() = default;

			virtual void create(const FRenderingStepCreateInfo& create_info)
			{
				m_bOneTime = create_info.oneTime;
			}

			virtual void execute(float dt)
			{
				if (!m_bNeedToBeDestroyed && m_bOneTime)
					m_bNeedToBeDestroyed = true;
			}

			bool expired()
			{
				return m_bNeedToBeDestroyed;
			}
		protected:
			bool m_bOneTime{ false };
			bool m_bNeedToBeDestroyed{ false };

			IRenderInterface* m_pRenderInterface{ nullptr };
		};

		class ISynchronizationStep : public IRenderGraphStep
		{
		public:
			virtual ~ISynchronizationStep() override = default;

			void create(const FRenderingStepCreateInfo& create_info) override
			{
				m_barrier = create_info.synchronization.barrier;

				_create(create_info);
				IRenderGraphStep::create(create_info);
			}

			void execute(float dt) override
			{
				_execute(dt);
				IRenderGraphStep::execute(dt);
			}

		protected:
			virtual void _create(const FRenderingStepCreateInfo& create_info) = 0;
			virtual void _execute(float dt) = 0;
		protected:
			FRenderingStepBarrier m_barrier{};
		};

		class IRenderingStep : public IRenderGraphStep
		{
		public:
			virtual ~IRenderingStep() override = default;

			void create(const FRenderingStepCreateInfo& create_info) override
			{
				// Save rendering info
				m_beginRenderingInfo.queue = create_info.rendering.queue;
				m_beginRenderingInfo.colorAttachments = create_info.rendering.renderTargets;
				m_beginRenderingInfo.depthAttachment = create_info.rendering.depthTarget;
				m_beginRenderingInfo.stencilAttachment = create_info.rendering.stencilTarget;
				m_beginRenderingInfo.flipViewport = create_info.rendering.flipViewport;

				m_sizePolicyType = create_info.rendering.sizeInfo.type;
				m_target = create_info.rendering.sizeInfo.target;

				if (m_sizePolicyType == ERenderingSizeType::eManual)
				{
					m_beginRenderingInfo.renderArea.size.x = create_info.compute.dispatchInfo.sizeX;
					m_beginRenderingInfo.renderArea.size.y = create_info.compute.dispatchInfo.sizeY;
				}

				_create(create_info);
				IRenderGraphStep::create(create_info);
			}

			void execute(float dt) override
			{
				if (m_sizePolicyType == ERenderingSizeType::eFromRenderTarget)
				{
					auto& render_context = m_pRenderInterface->getRenderContext();
					auto& info = render_context->getTextureInfo(m_target);

					m_beginRenderingInfo.renderArea.size.x = info.width;
					m_beginRenderingInfo.renderArea.size.y = info.height;
				}

				_execute(dt);

				IRenderGraphStep::execute(dt);
			}
		protected:
			virtual void _create(const FRenderingStepCreateInfo& create_info) = 0;
			virtual void _execute(float dt) = 0;
		protected:
			FBeginRenderingInfo m_beginRenderingInfo{};
			ERenderingSizeType m_sizePolicyType{};
			size_t m_target{ ~0ull };
		};

		class IComputeStep : public IRenderGraphStep
		{
		public:
			virtual ~IComputeStep() override = default;

			void create(const FRenderingStepCreateInfo& create_info) override
			{
				// Save rendering info
				m_beginComputeInfo.queue = create_info.compute.queue;

				for (const auto& [name, attachment] : create_info.compute.writeTargets)
					m_beginComputeInfo.writeAttachments.emplace_back(attachment);

				for (const auto& [name, buffer] : create_info.compute.writeBuffers)
					m_beginComputeInfo.writeBuffers.emplace_back(buffer);

				m_groupPolicyType = create_info.compute.dispatchInfo.type;
				m_target = create_info.compute.dispatchInfo.target;

				if (m_groupPolicyType == ERenderingSizeType::eManual)
				{
					m_beginComputeInfo.globalSizes.x = create_info.compute.dispatchInfo.sizeX;
					m_beginComputeInfo.globalSizes.y = create_info.compute.dispatchInfo.sizeY;
					m_beginComputeInfo.globalSizes.z = create_info.compute.dispatchInfo.sizeZ;
				}

				_create(create_info);
				IRenderGraphStep::create(create_info);
			}

			void execute(float dt) override
			{
				if (m_groupPolicyType == ERenderingSizeType::eFromRenderTarget)
				{
					auto& render_context = m_pRenderInterface->getRenderContext();
					auto& info = render_context->getTextureInfo(m_target);

					m_beginComputeInfo.globalSizes.x = info.width;
					m_beginComputeInfo.globalSizes.y = info.height;
					m_beginComputeInfo.globalSizes.z = info.depth;
				}

				_execute(dt);

				IRenderGraphStep::execute(dt);
			}
		protected:
			virtual void _create(const FRenderingStepCreateInfo& create_info) = 0;
			virtual void _execute(float dt) = 0;
		protected:
			FBeginComputeInfo m_beginComputeInfo{};

			ERenderingSizeType m_groupPolicyType{};
			size_t m_target{ ~0ull };
		};
	}
}