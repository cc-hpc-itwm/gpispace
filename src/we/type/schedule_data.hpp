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

#pragma once

#include <boost/optional.hpp>

namespace we
{
  namespace type
  {
    struct schedule_data
    {
    public:
      schedule_data() = default;
      schedule_data
        ( ::boost::optional<unsigned long> const& num_worker
        , ::boost::optional<unsigned long> const& maximum_number_of_retries
        );

      const ::boost::optional<unsigned long>& num_worker() const;
      ::boost::optional<unsigned long> const& maximum_number_of_retries() const;

    private:
      const ::boost::optional<unsigned long> _num_worker;
      const ::boost::optional<unsigned long> _max_num_retries;
    };
  }
}
