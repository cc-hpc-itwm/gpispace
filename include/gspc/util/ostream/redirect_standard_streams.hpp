#pragma once

#include <gspc/util/ostream/redirect.hpp>

#include <mutex>
#include <ostream>
#include <string>
#include <vector>



    namespace gspc::util::ostream
    {
      class redirect_standard_streams
      {
      public:
        redirect_standard_streams (std::vector<std::string>& lines);

      private:
        std::mutex _guard;
        std::vector<std::string>& _lines;
        struct appender : public redirect
        {
          appender ( std::ostream& os
                   , std::string prefix
                   , std::vector<std::string>& lines
                   , std::mutex& guard
                   );
          prepend_line _prepender;
        };
        appender _append_from_clog;
        appender _append_from_cout;
        appender _append_from_cerr;
      };
    }
