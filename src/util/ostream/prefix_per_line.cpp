#include <gspc/util/ostream/prefix_per_line.hpp>

#include <iostream>
#include <string>



    namespace gspc::util::ostream
    {
      prefix_per_line::prefix_per_line (std::string prefix, std::ostream& os)
        : line_by_line
          ( [&os, prefix] (std::string const& line)
            {
              os << prefix << line << std::endl;
            }
          )
        , std::ostream (this)
      {}
    }
