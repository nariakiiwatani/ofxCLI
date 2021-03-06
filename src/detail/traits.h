#pragma once

#include "json.hpp"
#include <tuple>

namespace ofx {
namespace cli {
namespace detail {
	
	template<typename Arg> inline static std::string get_type_name() { return "unknown_type"; }
	template<> inline std::string get_type_name<std::string>() { return "string"; }
	template<> inline std::string get_type_name<int>() { return "int"; }
	template<> inline std::string get_type_name<float>() { return "float"; }
	
	template<typename... Args>
	inline std::vector<std::string> get_tuple_element_name(std::tuple<Args...>) {
		return {(get_type_name<Args>())...};
	}

	template<typename F, typename R, typename... Args>
	auto make_tuple_from_args_list_impl(R(F::*f)(Args...) const) -> std::tuple<Args...>;
	
	template<typename F>
	auto make_tuple_from_args_list(F f)->decltype(make_tuple_from_args_list_impl(&F::operator()));
	
	template<typename Tuple, typename F, std::size_t ...I>
	void apply_impl(F &&f, const Tuple &args, nlohmann::detail::index_sequence<I...>) {
		f(std::get<I>(args)...);
	}
	template<typename... Args, typename F>
	void apply(F &&f, const std::tuple<Args...> &args) {
		apply_impl<std::tuple<Args...>>(f, args, nlohmann::detail::make_index_sequence<sizeof...(Args)>{});
	}
	template<std::size_t N, typename Tuple>
	typename std::tuple_element<N, Tuple>::type to_element(const std::vector<std::string> &arr, const Tuple &defaults) {
		return N < arr.size() ? ofFromString<typename std::tuple_element<N, Tuple>::type>(arr[N]) : std::get<N>(defaults);
	}
	template<typename Tuple, std::size_t... I>
	Tuple make_tuple_from_vector_impl(const std::vector<std::string> &arr, const Tuple &defaults, nlohmann::detail::index_sequence<I...>) {
		return std::make_tuple(to_element<I>(arr, defaults)...);
	}
	template<typename... Args>
	std::tuple<Args...> make_tuple_from_vector(const std::vector<std::string> &arr, const std::tuple<Args...> &defaults) {
		return make_tuple_from_vector_impl<std::tuple<Args...>>(arr, defaults,  nlohmann::detail::make_index_sequence<sizeof...(Args)>{});
	}
}
}}
