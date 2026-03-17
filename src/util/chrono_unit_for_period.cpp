#include <gspc/util/chrono_unit_for_period.hpp>


  namespace gspc::util
  {
#define CHRONO_UNIT_FOR_PREDEFINED_PERIOD(symbol_, ratio_)      \
    template<>                                                  \
      std::string chrono_unit_for_period<ratio_::period>()      \
      {                                                         \
        return #symbol_;                                        \
      }

    CHRONO_UNIT_FOR_PREDEFINED_PERIOD (ns, std::chrono::nanoseconds)
    CHRONO_UNIT_FOR_PREDEFINED_PERIOD (µs, std::chrono::microseconds)
    CHRONO_UNIT_FOR_PREDEFINED_PERIOD (ms, std::chrono::milliseconds)
    CHRONO_UNIT_FOR_PREDEFINED_PERIOD (s, std::chrono::seconds)
    CHRONO_UNIT_FOR_PREDEFINED_PERIOD (min, std::chrono::minutes)
    CHRONO_UNIT_FOR_PREDEFINED_PERIOD (hr, std::chrono::hours)

#undef CHRONO_UNIT_FOR_PREDEFINED_PERIOD
  }
