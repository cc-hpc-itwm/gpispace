// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
