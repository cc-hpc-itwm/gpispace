#include <gspc/util/chrono_unit_for_period.hpp>

#include <utility>



    namespace gspc::util::timer
    {
      template<typename Duration, typename Clock>
        scoped<Duration, Clock>::scoped ( std::string description
                                        , std::ostream& os
                                        )
          : _os (os)
          , _description (std::move (description))
          , _start (Clock::now())
      {
        _os << "START: " << _description << "...\n";
      }

      template<typename Duration, typename Clock>
        scoped<Duration, Clock>::~scoped()
      {
        auto const duration
          (std::chrono::duration_cast<Duration> (Clock::now() - _start));

        _os << "DONE: " << _description
            << " [" << duration.count()
            << ' '
            << gspc::util::chrono_unit_for_period<typename Duration::period>()
            << "]\n"
          ;
      }
    }
