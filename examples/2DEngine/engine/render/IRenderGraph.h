#pragma once

#include <edge/gfx/gfx_base.h>

namespace e2d {
	using RGResourceId = uint32_t;
	constexpr RGResourceId INVALID_RESOURCE_ID = 0;

	using RGResourceVersion = uint32_t;

	class RGResourceHandle {
	public:
		RGResourceHandle() = default;
		explicit RGResourceHandle(RGResourceId id) : resource_id_(id) {}

		[[nodiscard]] RGResourceId id() const noexcept { return resource_id_; }
		[[nodiscard]] bool is_valid() const noexcept { return resource_id_ != INVALID_RESOURCE_ID; }

		bool operator==(const RGResourceHandle& other) const noexcept {
			return resource_id_ == other.resource_id_;
		}

		bool operator!=(const RGResourceHandle& other) const noexcept {
			return !(*this == other);
		}

	private:
		RGResourceId resource_id_ = INVALID_RESOURCE_ID;
	};

	struct RGResource {
		RGResourceId id{ INVALID_RESOURCE_ID };
		RGResourceVersion version{ 0u };
		std::string name{};
		edge::gfx::ResourceType type{ edge::gfx::ResourceType::eUnknown };
		edge::gfx::ResourceStateFlags current_state{ edge::gfx::ResourceStateFlag::eUndefined };
		edge::gfx::ResourceStateFlags required_state{ edge::gfx::ResourceStateFlag::eUndefined };
		bool is_external{ false };
		bool is_persistent{ false };

		uint32_t first_use_pass{ UINT32_MAX };
		uint32_t last_use_pass{ 0 };

		size_t size_bytes = 0;
		bool needs_allocation = true;
	};

	struct RGBufferResource {
		edge::gfx::BufferCreateInfo create_info{};
		edge::gfx::IBuffer* buffer{ nullptr };

		RGBufferResource() = default;
		RGBufferResource(const edge::gfx::BufferCreateInfo& create_info) : create_info{ create_info } {}
	};

	struct RGTextureResource {
		edge::gfx::TextureCreateInfo create_info{};
		edge::gfx::ITexture* texture{ nullptr };

		RGTextureResource() = default;
		RGTextureResource(const edge::gfx::TextureCreateInfo& create_info) : create_info{ create_info } {}
	};

	struct RGResourceAccess {
		RGResourceId resource_id{ INVALID_RESOURCE_ID };
		edge::gfx::ResourceStateFlags required_state{};
		uint32_t subresource_index{ ~0u };
	};

	struct RGPassContext {
		edge::gfx::ICommandList* cmd_list{ nullptr };
		uint32_t pass_index{ 0 };
		std::string_view name{};

		std::vector<RGResourceAccess> resource_accesses;
	};

	template<typename _Fn>
	concept RGPassExecutor = requires(_Fn f, RGPassContext & ctx) {
		{ f(ctx) } -> std::same_as<void>;
	};

	class IRenderGraph {
	public:
	private:
	};
}