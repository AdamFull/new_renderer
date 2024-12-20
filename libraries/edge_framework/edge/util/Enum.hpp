#pragma once

#if (201711 <= __cpp_impl_three_way_comparison) && __has_include(<compare> )
#define HAS_SPACESHIP_OPERATOR
#endif
#if defined(HAS_SPACESHIP_OPERATOR)
#include <compare>
#endif

#include <array>
#include <type_traits>
#include <string_view>

#include "Serialization.hpp"

namespace edge
{
	template <typename FlagBitsType>
	struct FlagTraits
	{
	};

	template <typename BitType>
	class flags
	{
	public:
		using MaskType = typename std::underlying_type<BitType>::type;

		// constructors
		constexpr flags() noexcept : m_mask(0) {}

		constexpr flags(BitType bit) noexcept : m_mask(static_cast<MaskType>(bit)) {}

		constexpr flags(flags<BitType> const& rhs) noexcept = default;

		constexpr flags(MaskType flags) noexcept : m_mask(flags) {}

		// relational operators
#if defined(HAS_SPACESHIP_OPERATOR)
		auto operator<=>(flags<BitType> const&) const = default;
#else
		constexpr bool operator<(flags<BitType> const& rhs) const noexcept
		{
			return m_mask < rhs.m_mask;
		}

		constexpr bool operator<=(flags<BitType> const& rhs) const noexcept
		{
			return m_mask <= rhs.m_mask;
		}

		constexpr bool operator>(flags<BitType> const& rhs) const noexcept
		{
			return m_mask > rhs.m_mask;
		}

		constexpr bool operator>=(flags<BitType> const& rhs) const noexcept
		{
			return m_mask >= rhs.m_mask;
		}

		constexpr bool operator==(flags<BitType> const& rhs) const noexcept
		{
			return m_mask == rhs.m_mask;
		}

		constexpr bool operator!=(flags<BitType> const& rhs) const noexcept
		{
			return m_mask != rhs.m_mask;
		}
#endif

		// logical operator
		constexpr bool operator!() const noexcept
		{
			return !m_mask;
		}

		// bitwise operators
		constexpr flags<BitType> operator&(flags<BitType> const& rhs) const noexcept
		{
			return flags<BitType>(m_mask & rhs.m_mask);
		}

		constexpr flags<BitType> operator|(flags<BitType> const& rhs) const noexcept
		{
			return flags<BitType>(m_mask | rhs.m_mask);
		}

		constexpr flags<BitType> operator^(flags<BitType> const& rhs) const noexcept
		{
			return flags<BitType>(m_mask ^ rhs.m_mask);
		}

		constexpr flags<BitType> operator~() const noexcept
		{
			return flags<BitType>(m_mask ^ FlagTraits<BitType>::allflags);
		}

		// assignment operators
		constexpr flags<BitType>& operator=(flags<BitType> const& rhs) noexcept = default;

		constexpr flags<BitType>& operator|=(flags<BitType> const& rhs) noexcept
		{
			m_mask |= rhs.m_mask;
			return *this;
		}

		constexpr flags<BitType>& operator&=(flags<BitType> const& rhs) noexcept
		{
			m_mask &= rhs.m_mask;
			return *this;
		}

		constexpr flags<BitType>& operator^=(flags<BitType> const& rhs) noexcept
		{
			m_mask ^= rhs.m_mask;
			return *this;
		}

		// cast operators
		explicit constexpr operator bool() const noexcept
		{
			return !!m_mask;
		}

		explicit constexpr operator MaskType() const noexcept
		{
			return m_mask;
		}

	private:
		MaskType m_mask;
	};

	template<typename BitType>
	inline flags<BitType> and_(flags<BitType> l, flags<BitType> r)
	{
		return l & r;
	}

	template<typename BitType>
	inline flags<BitType> and_(flags<BitType> l, BitType r)
	{
		return l & r;
	}


	template<typename BitType>
	inline flags<BitType> or_(flags<BitType> l, flags<BitType> r)
	{
		return l | r;
	}

	template<typename BitType>
	inline flags<BitType> or_(flags<BitType> l, BitType r)
	{
		return l | r;
	}


	template<typename BitType>
	inline flags<BitType> xor_(flags<BitType> l, BitType r)
	{
		return l ^ r;
	}

	template<typename BitType>
	inline flags<BitType> xor_(flags<BitType> l, flags<BitType> r)
	{
		return l ^ r;
	}


	template<typename BitType>
	inline bool eq_(flags<BitType> l, flags<BitType> r)
	{
		return and_(l, r) == r;
	}

	template<typename BitType>
	inline bool eq_(flags<BitType> l, BitType r)
	{
		return and_(l, r) == r;
	}


	template<typename BitType>
	inline flags<BitType> not_(flags<BitType> f)
	{
		return ~f;
	}


	template <typename _Enum>
	inline constexpr std::array<std::pair<_Enum, std::string_view>, 0> enum_metadata;

	template <typename _Enum>
	constexpr bool has_enum_metadata = !enum_metadata<_Enum>.empty();

	template <typename _Enum>
	inline constexpr std::string_view enum_to_string(_Enum value)
	{
		for (const auto& [e, str] : enum_metadata<_Enum>)
		{
			if (e == value)
				return str;
		}

		return "";
	}

	template <typename _Enum>
	inline constexpr _Enum enum_from_string(std::string_view value)
	{
		for (const auto& [e, str] : enum_metadata<_Enum>)
		{
			if (value.compare(str) == 0)
				return e;
		}
		return static_cast<_Enum>(0);
	}

	template<class _Ty, class _ReturnType = typename std::underlying_type<_Ty>::type>
	inline constexpr _ReturnType enum_cast(_Ty enum_value)
	{
		return static_cast<_ReturnType>(enum_value);
	}
}

#if !defined(HAS_SPACESHIP_OPERATOR)
// relational operators only needed for pre C++20
template <typename BitType>
constexpr bool operator<(BitType bit, edge::flags<BitType> const& flags) noexcept
{
	return flags.operator>(bit);
}

template <typename BitType>
constexpr bool operator<=(BitType bit, edge::flags<BitType> const& flags) noexcept
{
	return flags.operator>=(bit);
}

template <typename BitType>
constexpr bool operator>(BitType bit, edge::flags<BitType> const& flags) noexcept
{
	return flags.operator<(bit);
}

template <typename BitType>
constexpr bool operator>=(BitType bit, edge::flags<BitType> const& flags) noexcept
{
	return flags.operator<=(bit);
}

template <typename BitType>
constexpr bool operator==(BitType bit, edge::flags<BitType> const& flags) noexcept
{
	return flags.operator==(bit);
}

template <typename BitType>
constexpr bool operator!=(BitType bit, edge::flags<BitType> const& flags) noexcept
{
	return flags.operator!=(bit);
}
#endif

// bitwise operators
template <typename BitType>
constexpr edge::flags<BitType> operator&(BitType bit, edge::flags<BitType> const& flags) noexcept
{
	return flags.operator&(bit);
}

template <typename BitType>
constexpr edge::flags<BitType> operator|(BitType bit, edge::flags<BitType> const& flags) noexcept
{
	return flags.operator|(bit);
}

template <typename BitType>
constexpr edge::flags<BitType> operator^(BitType bit, edge::flags<BitType> const& flags) noexcept
{
	return flags.operator^(bit);
}

#define EDGE_REGISTER_ENUM_CLASS_AS_FLAG_TYPE(FlagsName, EnumType) \
using FlagsName = edge::flags<EnumType>; \
inline FlagsName operator|(EnumType a, EnumType b) { return FlagsName(static_cast<FlagsName::MaskType>(a) | static_cast<FlagsName::MaskType>(b)); } \
inline FlagsName operator&(EnumType a, EnumType b) { return FlagsName(static_cast<FlagsName::MaskType>(a) & static_cast<FlagsName::MaskType>(b)); } \
inline FlagsName operator^(EnumType a, EnumType b) { return FlagsName(static_cast<FlagsName::MaskType>(a) ^ static_cast<FlagsName::MaskType>(b)); } \
inline EnumType operator~(EnumType e) { using underlying = std::underlying_type_t<EnumType>; return static_cast<EnumType>(~static_cast<underlying>(e)); }

#define EDGE_REGISTER_ENUM_CLASS(NameSpace, FlagType, ...) template<> inline constexpr std::array edge::enum_metadata<NameSpace::FlagType> = { __VA_ARGS__ }; \
namespace NameSpace {\
inline void to_json(nlohmann::json& j, const FlagType& t) { j = edge::enum_to_string(t); } \
inline void from_json(const nlohmann::json& j, FlagType& t) { t = edge::enum_from_string<FlagType>(j.get<std::string>().c_str()); } }

#define EDGE_REGISTER_ENUM_FLAGS(NameSpace, FlagsName, FlagType, ...) template<> inline constexpr std::array edge::enum_metadata<NameSpace::FlagType> = { __VA_ARGS__ }; \
namespace NameSpace {\
using FlagsName = edge::flags<FlagType>; \
inline FlagsName operator|(FlagType a, FlagType b) { return FlagsName(static_cast<FlagsName::MaskType>(a) | static_cast<FlagsName::MaskType>(b)); } \
inline FlagsName operator&(FlagType a, FlagType b) { return FlagsName(static_cast<FlagsName::MaskType>(a) & static_cast<FlagsName::MaskType>(b)); } \
inline FlagsName operator^(FlagType a, FlagType b) { return FlagsName(static_cast<FlagsName::MaskType>(a) ^ static_cast<FlagsName::MaskType>(b)); } \
inline FlagType operator~(FlagType e) { using underlying = std::underlying_type_t<FlagType>; return static_cast<FlagType>(~static_cast<underlying>(e)); } \
inline void to_json(nlohmann::json& j, const FlagType& t) { j = edge::enum_to_string(t); } \
inline void from_json(const nlohmann::json& j, FlagType& t) { t = edge::enum_from_string<FlagType>(j.get<std::string>().c_str()); } }

#define EDGE_ENUM_META(Enum, Str) std::pair{Enum, Str}