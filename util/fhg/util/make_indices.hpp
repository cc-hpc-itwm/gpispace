// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_UTIL_MAKE_INDICES_HPP
#define FHG_UTIL_MAKE_INDICES_HPP

namespace fhg
{
  namespace util
  {
    template<std::size_t...> struct indices;

    template<std::size_t, typename> struct make_indices_impl;
    template<std::size_t N, std::size_t... Indices>
      struct make_indices_impl<N, indices<Indices...>>
    {
      using type = typename make_indices_impl<N - 1, indices<N, Indices...>>::type;
    };
    template<std::size_t... Indices>
      struct make_indices_impl<0, indices<Indices...>>
    {
      using type = indices<0, Indices...>;
    };

    //! \note create indices<0, …, Size - 1>
    //! \note potentially slow for large N, and limited by -ftemplate-depth.
    template<std::size_t> struct make_indices_t;
    template<> struct make_indices_t<0>
    {
      using type = indices<>;
    };
    template<std::size_t Size> struct make_indices_t
    {
      using type = typename make_indices_impl<Size - 1, indices<>>::type;
    };

    template<std::size_t Size>
      using make_indices = typename make_indices_t<Size>::type;
  }
}

#endif
