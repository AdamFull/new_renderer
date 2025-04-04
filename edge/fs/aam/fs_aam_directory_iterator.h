#pragma once

#include "../fs_directory_iterator.h"

#include <stack>
#include <string>
#include <string_view>
#include <mutex>

struct AAssetManager;
struct AAssetDir;

namespace edge::fs {
	class AAMDirectoryIterator final : public IDirectoryIterator {
	public:
		AAMDirectoryIterator(AAssetManager* asset_manager, std::string_view path, bool recursive);
		~AAMDirectoryIterator() override;

		auto end() const noexcept -> bool override;
		auto next() -> void override;
		auto value() const noexcept -> std::string_view override;
	private:
        void advance();

		AAssetManager* asset_manager{ nullptr };

		std::string root{};
		bool recursive{ false };
        std::string current_entry_path;

        AAssetDir* current_dir{ nullptr };
        std::stack<std::pair<std::string, AAssetDir*>> dir_stack;

        mutable std::mutex mutex;
	};
}