#pragma once

#include <vector>
#include <string_view>

#include "ISynthesisNode.h"
#include "IAudioFile.h"

namespace e2d::audio {
	class FileInputNode final : public ISynthesisNode {
	public:
		~FileInputNode() override = default;

		[[nodiscard]] static auto construct(std::string_view path) -> std::unique_ptr<ISynthesisNode>;

		auto process(std::span<uint8_t> out, const AudioFormat& out_format) -> size_t override;
	private:
		auto load_file(std::string_view path) -> bool;
	private:
		std::unique_ptr<IAudioFile> file_;
		std::vector<uint8_t> file_bytes_{};
	};
}