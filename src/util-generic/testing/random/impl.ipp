// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

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
