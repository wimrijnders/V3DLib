#ifndef _LIB_SUPPORT_ASSIGN_H
#define _LIB_SUPPORT_ASSIGN_H

// implementation details, users never invoke these directly
namespace detail
{
    template <typename F, typename Tuple, bool Done, int Total, int... N>
    struct call_impl
    {
        static void call(F f, Tuple && t)
        {
            call_impl<F, Tuple, Total == 1 + sizeof...(N), Total, N..., sizeof...(N)>::call(f, std::forward<Tuple>(t));
        }
    };

    template <typename F, typename Tuple, int Total, int... N>
    struct call_impl<F, Tuple, true, Total, N...>
    {
        static void call(F f, Tuple && t)
        {
            f(std::get<N>(std::forward<Tuple>(t))...);
        }
    };
}

// user invokes this
template <typename F, typename Tuple>
void apply(F f, Tuple && t)
{
    typedef typename std::decay<Tuple>::type ttype;
    detail::call_impl<F, Tuple, 0 == std::tuple_size<ttype>::value, std::tuple_size<ttype>::value>::call(f, std::forward<Tuple>(t));
}

#endif  // _LIB_SUPPORT_ASSIGN_H
