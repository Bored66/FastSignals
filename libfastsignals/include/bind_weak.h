#pragma once
#include <type_traits>
#include <utility>
#include <functional>
#include <memory>

#include "std_ext_type.h"

namespace is
{
namespace signals
{
namespace detail
{
template <class ReturnType, class ClassType, bool AddConst, class... Args>
struct weak_binder
{
	using ConstMethodType = ReturnType (ClassType::*)(Args... args) const;
	using NonConstMethodType = ReturnType (ClassType::*)(Args... args);
    using MethodType = typename std::conditional<AddConst, ConstMethodType, NonConstMethodType>::type;
	using WeakPtrType = std::weak_ptr<ClassType>;
    using weak_binder_alias = typename detail::weak_binder<ReturnType, ClassType, AddConst, Args...>;
    //using type = ReturnType;

	weak_binder(MethodType pMethod, WeakPtrType&& pObject)
		: m_pMethod(pMethod)
		, m_pObject(pObject)
	{
	}

	ReturnType operator()(Args... args) const
	{
		if (auto pThis = m_pObject.lock())
		{
			return (pThis.get()->*m_pMethod)(std::forward<Args>(args)...);
		}
		return ReturnType();
	}
    auto bind(Args... args) -> decltype(std::bind(this, args...))
    {
        return std::bind(this, args...);
    }
    void bind_args(Args... args)
    {
        _tuple = {args...};
    }
    template<size_t... N>
    ReturnType makeCall(std::index_sequence<N...>)
    {
        return operator()(std::get<N>(_tuple)...);
    }
    ReturnType makeCall()
    {
        constexpr auto tuple_size = std::tuple_size<std::tuple<Args...>>::value;
        return makeCall(std::make_index_sequence<tuple_size>{});
    }
	MethodType m_pMethod;
	WeakPtrType m_pObject;
    std::tuple<typename std::remove_cv<typename std::remove_reference<Args>::type>::type...> _tuple;
};
} // namespace detail

/// Weak this binding of non-const methods.
template <typename ReturnType, typename ClassType, typename... Params, typename... Args>
auto
bind_weak(ReturnType (ClassType::*memberFn)(Params... args), std::shared_ptr<ClassType> const& pThis, Args... args)
->
decltype(detail::weak_binder<ReturnType, ClassType, false, Params...>(memberFn, pThis).bind(args...))
{
    return detail::weak_binder<ReturnType, ClassType, false, Params...>(memberFn, pThis).bind(args...);
}

/// Weak this binding of const methods.
template <typename ReturnType, typename ClassType, typename... Params, typename... Args>
auto bind_weak(ReturnType (ClassType::*memberFn)(Params... args) const, std::shared_ptr<ClassType> const& pThis, Args... args)
->
decltype(detail::weak_binder<ReturnType, ClassType, true, Params...>(memberFn, pThis).bind(args...))

{
    return detail::weak_binder<ReturnType, ClassType, true, Params...>(memberFn, pThis).bind(args...);
}

/// Weak this binding of non-const methods.
template <typename ReturnType, typename ClassType, typename... Params, typename... Args>
auto bind_weak(ReturnType (ClassType::*memberFn)(Params... args), std::weak_ptr<ClassType> pThis, Args... args)
->
decltype(detail::weak_binder<ReturnType, ClassType, false, Params...>(memberFn, std::move(pThis)).bind(args...))

{
    return detail::weak_binder<ReturnType, ClassType, false, Params...>(memberFn, std::move(pThis)).bind(args...);
}

/// Weak this binding of const methods.
template <typename ReturnType, typename ClassType, typename... Params, typename... Args>
auto bind_weak(ReturnType (ClassType::*memberFn)(Params... args) const, std::weak_ptr<ClassType> pThis, Args... args)
->
decltype(detail::weak_binder<ReturnType, ClassType, true, Params...>(memberFn, std::move(pThis)).bind(args...))

{
//	using weak_binder_alias = detail::weak_binder<ReturnType, ClassType, true, Params...>;

//	weak_binder_alias invoker(memberFn, std::move(pThis));
    return detail::weak_binder<ReturnType, ClassType, true, Params...>(memberFn, std::move(pThis)).bind(args...);
}

} } // namespace is::signals

namespace std  {

template<typename RetType, typename ClassType, bool Const>
struct result_of<is::signals::detail::weak_binder<RetType, ClassType, Const>*&()>
{  using type = RetType; };
/*
template<typename RetType, typename ClassType, bool Const>
struct __invoke_result<is::signals::detail::weak_binder<RetType, ClassType, Const>*&>
{  using type = RetType; };
*/
}
