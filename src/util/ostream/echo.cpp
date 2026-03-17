#include <gspc/util/ostream/echo.hpp>

#include <gspc/util/ostream/put_time.hpp>

#include <chrono>
#include <string>



    namespace gspc::util::ostream
    {
      echo::echo (std::ostream& os)
        : line_by_line
            ( [&os] (std::string const& line)
              {
                os << '[' << put_time<std::chrono::system_clock>() << ']'
                   << ' ' << line
                   << std::endl;
              }
            )
        , std::ostream (this)
      {}
    }
