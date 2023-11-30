#pragma once

#include <easygltf_base.h>

namespace easygltf
{
	inline size_t get_size_by_component_type(int32_t component_type)
	{
		switch (component_type)
		{
		case base::enumerators::component_type::byte_type:
		case base::enumerators::component_type::ubyte_type: return 1ull; break;
		case base::enumerators::component_type::short_type:
		case base::enumerators::component_type::ushort_type: return 2ull; break;
		case base::enumerators::component_type::int_type:
		case base::enumerators::component_type::uint_type:
		case base::enumerators::component_type::float_type: return 4ull; break;
		case base::enumerators::component_type::double_type: return 8ull; break;
		}

		return 0ull;
	}

	inline size_t get_element_count_by_accessor_type(const char* type)
	{
		if (!std::strcmp(type, base::enumerators::accessor_type::scalar_type))
			return 1ull;
		else if (!std::strcmp(type, base::enumerators::accessor_type::vec2_type))
			return 2ull;
		else if (!std::strcmp(type, base::enumerators::accessor_type::vec3_type))
			return 3ull;
		else if (!std::strcmp(type, base::enumerators::accessor_type::vec4_type))
			return 4ull;
		else if (!std::strcmp(type, base::enumerators::accessor_type::mat2_type))
			return 4ull;
		else if (!std::strcmp(type, base::enumerators::accessor_type::mat3_type))
			return 9ull;
		else if (!std::strcmp(type, base::enumerators::accessor_type::mat4_type))
			return 16ull;

		return 0ull;
	}

	inline size_t get_accessor_element_size(const Accessor& accessor)
	{
		size_t size = get_size_by_component_type(accessor.componentType);
		size_t count = get_element_count_by_accessor_type(accessor.type.c_str());
		return size * count;
	}
}