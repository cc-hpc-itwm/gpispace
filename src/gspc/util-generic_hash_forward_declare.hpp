#pragma once

#include <util-generic/hash/combined_hash.hpp>

#define UTIL_STD_HASH_DECLARE(type_)                                \
  namespace std                                                     \
  {                                                                 \
    template<>                                                      \
      struct hash<type_>                                            \
    {                                                               \
      std::size_t operator() (type_ const&) const;                  \
    };                                                              \
  }                                                                 \
  struct util_forward_declare_std_hash_semi_hack

#define UTIL_STD_HASH_DEFINE(type_, var_, how_...)                  \
  namespace std                                                     \
  {                                                                 \
    std::size_t hash<type_>::operator() (type_ const& var_) const   \
    {                                                               \
      return how_;                                                  \
    }                                                               \
  }                                                                 \
  struct util_forward_declare_std_hash_semi_hack

#define UTIL_MAKE_COMBINED_STD_HASH_DECLARE(type_)                  \
  UTIL_STD_HASH_DECLARE (type_)

#define UTIL_MAKE_COMBINED_STD_HASH_DEFINE(type_, var_, what_...)   \
  UTIL_STD_HASH_DEFINE                                              \
    (type_, var_, fhg::util::combined_hash (what_))
