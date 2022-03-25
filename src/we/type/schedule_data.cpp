// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

#include <we/type/schedule_data.hpp>

#include <stdexcept>

namespace we
{
  namespace type
  {
    schedule_data::schedule_data
      ( ::boost::optional<unsigned long> const& num_worker
      , ::boost::optional<unsigned long> const& maximum_number_of_retries
      )
        : _num_worker (num_worker)
        , _max_num_retries (maximum_number_of_retries)
    {
      if (!!_num_worker && _num_worker.get() == 0UL)
      {
        throw std::logic_error ("schedule_data: num_worker == 0UL");
      }
    }

    const ::boost::optional<unsigned long>& schedule_data::num_worker() const
    {
      return _num_worker;
    }

    ::boost::optional<unsigned long> const& schedule_data::maximum_number_of_retries() const
    {
      return _max_num_retries;
    }
  }
}
