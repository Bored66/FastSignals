#pragma once

#include "function_detail.h"

namespace is { namespace signals
{
// Derive your class from not_directly_callable to prevent function from wrapping it using its template constructor
// Useful if your class provides custom operator for casting to function
struct not_directly_callable
{
};

template <class Fn, class Function, class Return, class... Arguments>
using enable_if_callable_t = typename std::enable_if
    <
    !std::is_same<typename std::decay<Fn>::type, Function>::value &&
    !std::is_base_of<not_directly_callable, typename std::decay<Fn>::type>::value &&
    std::is_same<typename std::invoke_result<Fn, Arguments...>::type, Return>::value>::type;

template <class Signature>
class function;

// Compact function class - causes minimal code bloat when compiled.
// Replaces std::function in this library.
template <class Return, class... Arguments>
class function<Return(Arguments...)>
{
public:
	function() = default;

	function(const function& other) = default;
	function(function&& other) noexcept = default;
	function& operator=(const function& other) = default;
	function& operator=(function&& other) noexcept = default;

	template <class Fn, typename = enable_if_callable_t<Fn, function<Return(Arguments...)>, Return, Arguments...>>
    function(Fn&& function) noexcept(detail::is_noexcept_packed_function_init<Fn, Return, Arguments...>())
	{
		m_packed.init<Fn, Return, Arguments...>(std::forward<Fn>(function));
	}

	Return operator()(Arguments&&... args) const
	{
		auto& proxy = m_packed.get<Return(Arguments...)>();
		return proxy(std::forward<Arguments>(args)...);
	}

	detail::packed_function release() noexcept
	{
		return std::move(m_packed);
	}

private:
	detail::packed_function m_packed;
};

} } // namespace is::signals
