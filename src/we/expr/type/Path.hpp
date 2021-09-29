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
      Path (Path const&) = default;
      Path (Path&&) = default;
      Path& operator= (Path const&) = default;
      Path& operator= (Path&&) = default;

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
