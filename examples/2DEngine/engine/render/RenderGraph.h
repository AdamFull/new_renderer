#pragma once

#include "IRenderGraph.h"

namespace e2d {
	// Helper class for mae pass description
	class PassDescriptor {
	public:
		PassDescriptor(std::string_view name);

		template<RGPassExecutor _Fn>
		auto execute(_Fn&& executor) -> PassDescriptor {
			executor_ = std::forward<F>(executor);
			return *this;
		}

		[[nodiscard]] auto name() const noexcept -> std::string const& { return name_; }
		[[nodiscard]] const std::vector<RGResourceAccess>& resource_accesses() const noexcept { return resource_accesses_; }
	private:
		std::string name_;
		std::vector<RGResourceAccess> resource_accesses_;
		std::function<void(RGPassContext&)> executor_;
	};

	class RenderGraph final : IRenderGraph {
	public:

	};
}