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

#include <boost/optional.hpp>

#include <cstddef>
#include <exception>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace fhg
{
  namespace util
  {
    //! Call \a fun for all elements in the range given by \a begin
    //! and \a end with an iterator pair of \a block_size elements
    //! each time. Every element is only given once.
    //! \note If no \a block_size is given, all elements are shown at once.
    //! \note The last callback invocation may differ in count.
    //! \note \a block_size shall be positive if given.
    template<typename Fun, typename It>
      void blocked ( It begin, It end
                   , boost::optional<std::size_t> const& block_size
                   , Fun&& fun
                   );

    //! Call \a fun for all elements in \a container with up to \a
    //! block_size threads in parallel. The results are collected and
    //! returned, indexed by a key determined by calling \a key on the
    //! respective element. If there is an exception thrown for one of
    //! the invocations, that exception is returned instead.
    template<typename Key, typename Result, typename Fun, typename Container>
      std::pair< std::unordered_map<Key, Result>
               , std::unordered_map<Key, std::exception_ptr>
               >
        blocked_async_with_results
          ( Container const& container
          , boost::optional<std::size_t> const& block_size
          , std::function<Key (typename Container::const_iterator::value_type const&)>
              const& key
          , Fun&& fun
          );

    //! Call \a fun for all elements in \a container with up to \a
    //! block_size threads in parallel. The results are discarded, but
    //! success or failure is recorded and returned indexed by a key
    //! determined by calling \a key on the respective element.
    template<typename Key, typename Fun, typename Container>
      std::pair< std::unordered_set<Key>
               , std::unordered_map<Key, std::exception_ptr>
               >
        blocked_async
          ( Container const& container
          , boost::optional<std::size_t> const& block_size
          , std::function<Key (typename Container::const_iterator::value_type const&)>
              const& key
          , Fun&& fun
          );
  }
}

#include <util-generic/blocked.ipp>
