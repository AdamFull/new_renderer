#pragma once

#include <glm/glm.hpp>

#include <array>
#include <vector>
#include <string>

#include <map>
#include <unordered_map>

namespace edge::utl {
	template<typename T> struct is_std_array : std::false_type {};
	template<typename T, std::size_t N> struct is_std_array<std::array<T, N>> : std::true_type {};

	template<typename T> struct is_std_vector : std::false_type {};
	template<typename... Ts> struct is_std_vector<std::vector<Ts...>> : std::true_type {};

	template<typename T> struct is_std_map : std::false_type {};
	template<typename... Ts> struct is_std_map<std::map<Ts...>> : std::true_type {};
	template<typename... Ts> struct is_std_map<std::unordered_map<Ts...>> : std::true_type {};

	template<typename T> struct is_std_container : std::false_type {};
	template<typename... Ts> struct is_std_container<std::vector<Ts...>> : std::true_type {};
	template<typename... Ts> struct is_std_container<std::map<Ts...>> : std::true_type {};
	template<typename... Ts> struct is_std_container<std::unordered_map<Ts...>> : std::true_type {};

	template<typename _Ty> struct is_glm_vec : std::false_type {};
	template<glm::length_t N, typename _Ty, glm::qualifier Q>
	struct is_glm_vec<glm::vec<N, _Ty, Q>> : std::true_type {};

	template<typename _Ty> struct is_glm_quat : std::false_type {};
	template<typename _Ty, glm::qualifier _Q>
	struct is_glm_quat<glm::qua<_Ty, _Q>> : std::true_type {};
}