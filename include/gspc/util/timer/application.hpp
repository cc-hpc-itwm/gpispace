#pragma once

#include <gspc/util/ostream/echo.hpp>
#include <gspc/util/timer/sections.hpp>

#include <chrono>
#include <iostream>
#include <string>


  namespace gspc::util
  {
    namespace timer
    {
      //! This example usage
      //!
      //! ```c++
      //! int main()
      //! {
      //!   gspc::util::default_application_timer out {"holla"};
      //!   FHG_UTIL_FINALLY ([&] { out << "bye" << std::endl; });
      //!
      //!   out << "welcome" << std::endl;
      //!
      //!   out.section ("yippie");
      //!   out << "message" << std::endl;
      //!
      //!   out.section ("yeah");
      //!   out << "fun fun fun" << std::endl;
      //!
      //!   out.section ("i go home");
      //! }
      //! ```
      //!
      //! will produce output (on std::cout) similar to this:
      //!
      //! ```
      //! [2018-08-30 10:26:55] START: holla...
      //! [2018-08-30 10:26:55] welcome
      //! [2018-08-30 10:26:55] START: yippie...
      //! [2018-08-30 10:26:55] message
      //! [2018-08-30 10:26:55] DONE: yippie [2 ms]
      //! [2018-08-30 10:26:55] START: yeah...
      //! [2018-08-30 10:26:55] fun fun fun
      //! [2018-08-30 10:26:55] DONE: yeah [0 ms]
      //! [2018-08-30 10:26:55] START: i go home...
      //! [2018-08-30 10:26:55] bye
      //! [2018-08-30 10:26:55] DONE: i go home [9 ms]
      //! [2018-08-30 10:26:55] DONE: holla [12 ms]
      //! ```

      template<typename Duration, typename Clock = std::chrono::steady_clock>
        struct application : public ostream::echo
                           , public sections<Duration, Clock>
      {
        application (std::string, std::ostream& = std::cout);
      };
    }

    using default_application_timer
      = timer::application<std::chrono::milliseconds>;
  }


#include <gspc/util/timer/application.ipp>
