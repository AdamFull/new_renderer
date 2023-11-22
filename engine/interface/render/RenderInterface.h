#pragma once

#include <interface/SharedStructure.h>
#include <interface/render/BaseEnumerators.h>
#include <interface/render/BufferStructures.h>
#include <interface/render/LightStructures.h>
#include <interface/render/MaterialStructures.h>
#include <interface/render/RenderingStructures.h>
#include <interface/render/ShaderStructures.h>
#include <interface/render/TextureStructures.h>
#include <interface/render/VertexStructures.h>

#include <memory>

namespace engine
{
	namespace render
	{
		struct FGlobalUniformCreateInfo
		{
			std::unordered_map<size_t, FReflectionUniformBlock<FReflectionUniform>> uniforms{};
		};

		struct FGlobalUniformUpdateInfo
		{
			std::unordered_map<size_t, FUniformDataView> uniforms{};
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
			virtual void bindTexture(const char* name, size_t texture_resource) = 0;

			virtual void flushDescriptors() = 0;
			virtual void flushConstants(uint32_t queue) = 0;
			virtual void bind(uint32_t queue) = 0;
		};

		class IRenderMaterial
		{
		public:
			virtual ~IRenderMaterial() = default;

			virtual void addTexture(const char* name, size_t texture_id) = 0;
			virtual void bind() = 0;
		};

		class IRenderingStep
		{
		public:
			virtual ~IRenderingStep() = default;

			virtual void create(const FRenderingStepCreateInfo& create_info) = 0;
			virtual void execute(float dt) = 0;
		};

		struct FRenderCamera
		{
			float fov{ 45.f };
			float nearp{ 0.1f };
			float farp{ 1024.f };

			glm::vec3 viewPos{ 0.f };
			glm::mat4 view{ 1.f }, invView{ 1.f }, projection{ 1.f }, invProjection{ 1.f }, viewProjection{ 1.f }, invViewProjection{ 1.f };

			glm::vec3 up{ 0.f };
			glm::vec3 forward{ 0.f };
			glm::vec3 right{ 0.f };

			FFrustum frustum{};
		};

		struct FRenderNode
		{
			glm::mat4 model{};
		};

		struct FRenderDataPacket
		{
			FRenderCamera camera{};

			std::vector<FGeometryBatch> sceneBatches{};

			std::unordered_map<uint32_t, FRenderNode> nodes{};
			std::vector<std::array<glm::mat4, 128>> skins{};

			// Lights data
			FDirectionalLightsData directionalLights{};
			FPointLightsData pointLights{};
			FSpotLightsData spotLights{};
		};

		class IRenderContext
		{
		public:
			virtual ~IRenderContext() = default;

			virtual void create(const FRenderContextCreateInfo& context_ci) = 0;
			virtual void createGlobalDescriptor(const FGlobalUniformCreateInfo& uniform_ci) = 0;
			virtual void updateGlobalDescriptorData(const FGlobalUniformUpdateInfo& data_view) = 0;

			virtual size_t createTexture(const std::string& name, const FTextureCreateInfo& texture_ci, FTextureUploadInfo&& upload_info) = 0;
			virtual size_t createRenderTargets(const std::string& name, const FTextureCreateInfo& texture_ci) = 0;
			virtual void destroyTexture(size_t resource_id) = 0;

			virtual size_t createShader(const std::string& name, const FGraphicsShaderCreateInfo& shader_ci, const  std::vector<FShaderCreationBlockInfo>& shader_stages) = 0;
			virtual size_t createShader(const std::string& name, const FComputeShaderCreateInfo& shader_ci, const  std::vector<FShaderCreationBlockInfo>& shader_stages) = 0;
			virtual FShaderReflectionData getShaderReflection(const size_t& shader_view) = 0;
			virtual void updateShaderResources(const FShaderResourceCommit& resources, uint32_t /*EShaderResourcesUpdateFlagBits*/ flags = SHADER_RESOURCES_UPDATE_ALL_FLAG_BIT) = 0;
			virtual void flushConstantRanges(size_t shader_id, const std::unordered_map<size_t, FUniformDataView>& constants, uint32_t queue_type) = 0;
			virtual void bindShader(size_t shader_id, uint32_t queue_type) = 0;
			virtual void destroyShader(size_t resource_id) = 0;

			virtual size_t createBuffer(const std::string& name, const FBufferCreateInfo& buffer_ci) = 0;
			virtual void uploadDataToBuffer(const FBufferUploadInfo& upload_info) = 0;
			virtual FBufferView getBufferView(size_t buffer_id) = 0;
			virtual void bindVertexBuffers(const FVertexBufferBindingInfo& binding_info, uint32_t queue) = 0;
			virtual void bindIndexBuffer(const FIndexBufferBindingInfo& binding_info, uint32_t queue) = 0;
			virtual void mapBufferMemory(size_t buffer_id, void*& memory) = 0;
			virtual void unmapBufferMemory(size_t buffer_id) = 0;
			virtual void flushBuffer(size_t buffer_id, size_t size, size_t offset) = 0;
			virtual void destroyBuffer(size_t resource_id) = 0;

			virtual void beginRendering(const FBeginRenderingInfo& rendering_info) = 0;
			virtual void endRendering(const FBeginRenderingInfo& rendering_info) = 0;

			virtual void setViewport(float width, float height, bool flip_viewport) = 0;
			virtual void setScissorRect(int32_t offset_x, int32_t offset_y, uint32_t width, uint32_t height) = 0;

			virtual void draw(uint32_t queue_type, const FGeometryDrawInfo& draw_info) = 0;

			virtual size_t getTextureResource(const char* name) = 0;
			virtual size_t getShaderResource(const char* name) = 0;

			virtual FPoint2D<uint32_t> getRenderResolution() = 0;

			virtual uint32_t getDepthFormat(bool depth_only) = 0;
			virtual uint32_t getApiFormat(uint32_t global_format) = 0;
			virtual uint32_t getPresentFormat() = 0;

			virtual void updateSurface(int32_t surface_width, int32_t surface_height) = 0;

			virtual void beginFrameCommandBuffers() = 0;
			virtual void endFrameCommandBuffers() = 0;

			virtual void beginDebugLabel(const std::string& label, uint32_t queue_type, uint32_t color = 0xffffffff) = 0;
			virtual void insertDebugLabel(const std::string& label, uint32_t queue_type, uint32_t color = 0xffffffff) = 0;
			virtual void endDebugLabel(uint32_t queue_type) = 0;
		};

		class IRenderInterface
		{
		public:
			virtual ~IRenderInterface() = default;

			virtual void create(const FRenderInterfaceCreateInfo& render_interface_ci) = 0;
			virtual void render(float dt) = 0;

			virtual size_t createTexture(const std::string& path) = 0;

			virtual size_t createShader(const std::string& name_info, const FGraphicsShaderCreateInfo& create_info) = 0;
			virtual size_t createShader(const std::string& name_info, const FComputeShaderCreateInfo& create_info) = 0;

			virtual size_t createMaterial(const std::string& name, const FMaterialCreateInfo& material_ci) = 0;

			virtual size_t createVertexHandler(const std::string& name, const FVertexHandlerCreateInfo& handler_ci) = 0;
			virtual const std::unique_ptr<IVertexHandler>& getVertexHandler(size_t id) = 0;

			virtual FShaderUploadInfo getShaderNameTag(const std::string& name) = 0;
			virtual void createRendering(const FRenderingStepCreateInfo& rendering_ci) = 0;

			virtual void updateSurfaceSize(int32_t width, int32_t height) = 0;

			virtual const std::unique_ptr<FRenderDataPacket>& getRenderPacket() = 0;

			virtual IRenderContext* getRenderContext() = 0;
		};
	}
}