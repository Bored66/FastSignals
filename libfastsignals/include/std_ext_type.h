#pragma once
#include <cstdint>
#include <type_traits>
#include <utility>

namespace std  {
template< class T, T... Ints >
struct integer_sequence
{
//public:
    static constexpr std::size_t size() noexcept
    { return sizeof...(Ints); }
    typedef T value_type;
    using type = integer_sequence;
    static_assert(
      std::is_integral<T>::value,
      "std::integer_sequence can only be instantiated with an integral type" );
};
template<std::size_t... Ints>
using index_sequence = std::integer_sequence<std::size_t, Ints...>;
// --------------------------------------------------------------

template <class Sequence1, class Sequence2>
struct _merge_and_renumber;

template <std::size_t... I1, std::size_t... I2>
struct _merge_and_renumber<index_sequence<I1...>, index_sequence<I2...>>
  : index_sequence<I1..., (sizeof...(I1)+I2)...>
{ };

// --------------------------------------------------------------

template <std::size_t N>
struct make_index_sequence
  : _merge_and_renumber<typename make_index_sequence<N/2>::type,
                        typename make_index_sequence<N - N/2>::type>
{
    using base = _merge_and_renumber<typename make_index_sequence<N/2>::type,
    typename make_index_sequence<N - N/2>::type>;
};

template<> struct make_index_sequence<0> : index_sequence<> {};// { using base = index_sequence<>; };
template<> struct make_index_sequence<1> : index_sequence<0>{};// { using base = index_sequence<0>; };
/* end of borrowed code */
//template<> struct make_index_sequence<2> : index_sequence<0,1> { };
//template<> struct make_index_sequence<3> : index_sequence<0,1,2> { };

template<class T, T N>
using make_integer_sequence = integer_sequence<T, N>;

template <std::size_t N, class T>
struct holder
{
    T value;
};

template <class I, class ...Tail>
struct tuple_holder_base;

template <std::size_t... I, class ...Tail>
struct tuple_holder_base< std::index_sequence<I...>, Tail... >
    : public holder<I , Tail>...
{
    static constexpr std::size_t size_v = sizeof...(I);

    constexpr tuple_holder_base() noexcept = default;

    constexpr tuple_holder_base(Tail... v) noexcept
        : holder<I, Tail>{ v }...
    {}
};


template <class ...Values>
using tuple_holder_seq =
    tuple_holder_base<std::make_index_sequence<sizeof...(Values)>, Values...>;

template <>
struct tuple_holder_base<std::index_sequence<0> > {
    static constexpr std::size_t size_v = 0;
};

template <class ...Values >
struct seq_tuple: tuple_holder_seq<Values...>
   // tuple_holder_base<std::make_index_sequence<sizeof...(Values)>, Values...>
{
    using tuple_holder_base<
    std::make_index_sequence<sizeof...(Values)>,
    Values...
    >::tuple_holder_base;
};



template< bool B, class T = void >
using enable_if_t = typename enable_if<B,T>::type;

template< class T >
using remove_reference_t = typename remove_reference<T>::type;

template< class T >
using remove_all_extents_t = typename remove_all_extents<T>::type;

template< class T >
using add_const_t    = typename add_const<T>::type;

template< std::size_t Len, std::size_t Align =
          __alignof__(typename __aligned_storage_msa<Len>::__type) >
using aligned_storage_t = typename aligned_storage<Len, Align>::type;

namespace detail
{
template <class T>
struct is_reference_wrapper : std::false_type {};
template <class U>
struct is_reference_wrapper<std::reference_wrapper<U>> : std::true_type {};

template<class T>
struct invoke_impl {
    template<class F, class... Args>
    static auto call(F&& f, Args&&... args)
        -> decltype(std::forward<F>(f)(std::forward<Args>(args)...));
};

template<class B, class MT>
struct invoke_impl<MT B::*> {
    template<class T, class Td = typename std::decay<T>::type,
        class = typename std::enable_if<std::is_base_of<B, Td>::value>::type
    >
    static auto get(T&& t) -> T&&;

    template<class T, class Td = typename std::decay<T>::type,
        class = typename std::enable_if<is_reference_wrapper<Td>::value>::type
    >
    static auto get(T&& t) -> decltype(t.get());

    template<class T, class Td = typename std::decay<T>::type,
        class = typename std::enable_if<!std::is_base_of<B, Td>::value>::type,
        class = typename std::enable_if<!is_reference_wrapper<Td>::value>::type
    >
    static auto get(T&& t) -> decltype(*std::forward<T>(t));

    template<class T, class... Args, class MT1,
        class = typename std::enable_if<std::is_function<MT1>::value>::type
    >
    static auto call(MT1 B::*pmf, T&& t, Args&&... args)
        -> decltype((invoke_impl::get(std::forward<T>(t)).*pmf)(std::forward<Args>(args)...));

    template<class T>
    static auto call(MT B::*pmd, T&& t)
        -> decltype(invoke_impl::get(std::forward<T>(t)).*pmd);
};

template<class F, class... Args, class Fd = typename std::decay<F>::type>
auto INVOKE(F&& f, Args&&... args)
    -> decltype(invoke_impl<Fd>::call(std::forward<F>(f), std::forward<Args>(args)...));


// Minimal C++11 implementation:
//template <class> struct result_of;
//template <class F, class... ArgTypes>
//struct result_of<F(ArgTypes...)>
//{
//    using type = decltype(detail::INVOKE(std::declval<F>(), std::declval<ArgTypes>()...));
//};

// Conforming C++14 implementation (is also a valid C++11 implementation):

template <typename AlwaysVoid, typename, typename...>
struct invoke_result;
template <typename F, typename...Args>
struct invoke_result<decltype(void(detail::INVOKE(std::declval<F>(), std::declval<Args>()...))),
                 F, Args...>
{
    using type = decltype(detail::INVOKE(std::declval<F>(), std::declval<Args>()...));
};
} // namespace detail

template <class> struct result_of;
//template <class F, class... ArgTypes>
//struct result_of<F(ArgTypes...)> : detail::invoke_result<void, F, ArgTypes...> {};

template <class F, class... ArgTypes>
struct invoke_result : detail::invoke_result<void, F, ArgTypes...> {};

}

