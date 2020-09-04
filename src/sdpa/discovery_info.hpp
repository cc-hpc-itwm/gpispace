// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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

#include <sdpa/job_states.hpp>
#include <sdpa/types.hpp>

#include <boost/optional.hpp>

#include <set>

namespace sdpa
{
  struct discovery_info_t;

  using discovery_info_set_t = std::set<discovery_info_t>;

  struct discovery_info_t
  {
    discovery_info_t () = default;
    discovery_info_t ( job_id_t job_id
                     , boost::optional<status::code> state
                     , discovery_info_set_t children
                     )
      : _job_id (job_id)
      , _state (state)
      , _children (children)
    {}

    job_id_t const& job_id() const
    {
      return _job_id;
    }
    boost::optional<status::code> state() const
    {
      return _state;
    }
    discovery_info_set_t children() const
    {
      return _children;
    }
    void add_child_info (discovery_info_t const& child_info)
    {
      _children.insert (child_info);
    }

    template<class Archive>
      void serialize (Archive &ar, unsigned int const)
    {
      ar & _job_id;
      ar & _state;
      ar & _children;
    }

    bool operator< (discovery_info_t const& other) const
    {
      return _job_id < other.job_id();
    }

  private:
    job_id_t _job_id;
    boost::optional<status::code> _state;
    discovery_info_set_t _children;
  };
}
