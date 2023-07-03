// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-generic/testing/random/impl.hpp>

#include <cstdint>
#include <random>

namespace fhg
{
  namespace util
  {
    namespace testing
    {
      namespace detail
      {
        using RandomNumberEngine = std::default_random_engine;

        //! A global random number engine, initialized with enough
        //! entropy.
        //! \note A FHG_UTIL_TESTING_OVERRIDE_SEED compile definition
        //! can be given to fixate the seed.
        RandomNumberEngine& GLOBAL_random_engine();
      }

      //! A functor that generates a given \a T when called.
      //! \note Specializations may exist for some types, allowing for
      //! e.g. minimum and maximum, or excluding specific values.
      //! \note See util-generic/testing/random/xxx.hpp for built-in
      //! specializations. These are automatically included, but moved
      //! there for readability.
      //! \note See util-generic/testing/random/impl.hpp for
      //! detail::random_impl and helpers/details on how to customize
      //! the framework.
      template<typename T>
        using random = detail::random_impl<T, void>;

      //! Generate a given number \a count random values filling the
      //! given \a Container type.
      //! \note The type to generate is implicitly given via the
      //! container type's `value_type`.
      template< typename Container
              , typename Generator = random<typename Container::value_type>
              >
        Container randoms (std::size_t count, Generator&& generator = {});

      //! A stateful functor that will ensure that every invocation
      //! returns a not previously rolled value.
      //! \note Will busy-stall when no more unique values can be
      //! generated.
      template<typename T, typename Generator = random<T>>
        struct unique_random;

      //! Convenience for `randoms<Container<T>, unique_random<T>>`:
      //! Generate a given number \a count unique random values
      //! filling the given \a Container type.
      //! \note The type to generate is implicitly given via the
      //! container type's `value_type`.
      //! \note Will busy-stall if not enough unique values can be
      //! generated.
      template<typename Container>
        Container unique_randoms (std::size_t count);
    }
  }
}

#include <util-generic/testing/random.ipp>
