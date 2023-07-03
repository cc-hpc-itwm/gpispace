// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#define FHG_UTIL_TESTING_RANDOM_SPECIALIZE_COMMON_DECL(type_, cond_, templ_)   \
  namespace fhg                                                                \
  {                                                                            \
    namespace util                                                             \
    {                                                                          \
      namespace testing                                                        \
      {                                                                        \
        namespace detail                                                       \
        {                                                                      \
          template<templ_> struct random_impl<type_, cond_>                    \
          {                                                                    \
            type_ operator()() const;                                          \
          };                                                                   \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }

#define FHG_UTIL_TESTING_RANDOM_SPECIALIZE_SIMPLE_IMPL(type_)                  \
  FHG_UTIL_TESTING_RANDOM_SPECIALIZE_COMMON_DECL(type_, void, )                \
  inline type_ fhg::util::testing::detail::random_impl<type_, void>            \
    ::operator()() const

#define FHG_UTIL_TESTING_RANDOM_SPECIALIZE_FULL_IMPL(type_, cond_, templ_...)  \
  FHG_UTIL_TESTING_RANDOM_SPECIALIZE_COMMON_DECL(type_, cond_, templ_)         \
  template<templ_>                                                             \
    inline type_ fhg::util::testing::detail::random_impl<type_, cond_>         \
      ::operator()() const
