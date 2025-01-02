// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <iosfwd>
#include <list>
#include <string>
#include <type_traits>

namespace expr
{
  namespace type
  {
    //! A path into the universe of types.
    struct Path
    {
    public:
      using Particle = std::string;
      using Particles = std::list<Particle>;

      template<typename... Args>
        using ParticlesArgs = std::is_constructible<Particles, Args...>;

      template< typename... Args
              , typename
              = typename std::enable_if_t<ParticlesArgs<Args...>::value>
              >
        Path (Args&&... args);

      Path (Particle particle);

      using const_iterator = Particles::const_iterator;
      const_iterator begin() const;
      const_iterator end() const;

      void emplace_back (Particle particle);
      void pop_back();

      friend std::ostream& operator<< (std::ostream& os, Path const& path);

    private:
      Particles _particles;
    };
  }
}

#include <we/expr/type/Path.ipp>
