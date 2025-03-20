#pragma once

#include <string_view>

namespace edge::fs
{
	class IDirectoryIterator {
	public:
		virtual ~IDirectoryIterator() = default;

		virtual auto end() const noexcept -> bool = 0;
		virtual auto next() -> void = 0;
		virtual auto value() const noexcept -> std::string_view = 0;
	};

	class DirectoryIterator {
	public:
		DirectoryIterator(IDirectoryIterator* iterator_impl) noexcept :
			impl{ iterator_impl } {
		}

		~DirectoryIterator() {
			if (--ref_cnt <= 0 && impl) {
				delete impl;
				impl = nullptr;
			}
		}

		DirectoryIterator(const DirectoryIterator& other) :
			impl{ other.impl },
			ref_cnt{ other.ref_cnt + 1 } {
		}

		[[nodiscard]] auto begin() -> DirectoryIterator& {
			return *this;
		}

		[[nodiscard]] auto end() -> DirectoryIterator& {
			static DirectoryIterator end_{ nullptr };
			return end_;
		}

		auto operator++() -> DirectoryIterator& {
			impl->next();
			return *this;
		}

		auto operator*() const noexcept -> std::string_view {
			return impl->value();
		}

		auto operator!=(const DirectoryIterator& other) -> bool const {
			//if (!other.impl)
			//	return !impl->End();
			//
			//return impl != other.impl;
			return impl && !impl->end();
		}
	private:
		IDirectoryIterator* impl{ nullptr };
		int32_t ref_cnt{ 1 };
	};
}