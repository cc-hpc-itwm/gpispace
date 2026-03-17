#pragma once

#include <string>


  namespace gspc::util
  {
    //! Determine a human readable string representing the unit for
    //! the given \c std::chrono::duration::period. SI units are
    //! represented by their standard symbols. Otherwise, the period
    //! is represented as "[n]s" or "[n/d]s" if d != 1.
    template<typename Period>
      std::string chrono_unit_for_period();
  }


#include <gspc/util/chrono_unit_for_period.ipp>
