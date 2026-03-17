#ifndef FHG_UTIL_BOOST_PROGRAM_OPTIONS_VALIDATORS_EXECUTABLE_HPP
#define FHG_UTIL_BOOST_PROGRAM_OPTIONS_VALIDATORS_EXECUTABLE_HPP

#include <gspc/util/boost/program_options/validators.hpp>

#include <gspc/util/boost/program_options/validators/existing_path.hpp>

#include <filesystem>
#include <gspc/util/fmt/std/filesystem/path.formatter.hpp>
#include <fmt/core.h>
#include <system_error>
#include <unistd.h>




      namespace gspc::util::boost::program_options
      {
        struct executable
        {
          executable (std::string const& option)
            : executable {std::filesystem::path {option}}
          {}

          executable (std::filesystem::path const& path)
            : _path {path}
          {
            int const rc (::access (_path.c_str(), F_OK | X_OK));
            int const errno_value (errno);
            if (rc == -1)
            {
              throw ::boost::program_options::invalid_option_value
                { fmt::format
                    ( "{} is not executable: {}"
                    , *this
                    , std::system_category().message (errno_value)
                    )
                };
            }
          }

          operator std::filesystem::path() const
          {
            return _path;
          }

        private:
          std::filesystem::path _path;
        };

        inline void validate ( ::boost::any& v
                             , const std::vector<std::string>& values
                             , executable*
                             , int
                             )
        {
          validate<executable> (v, values);
        }

        inline auto operator<<
          ( std::ostream& os
          , executable const& p
          ) -> std::ostream&
        {
          return os << static_cast<std::filesystem::path> (p);
        }
      }




#include <fmt/ostream.h>

namespace fmt
{
  template<>
    struct formatter<gspc::util::boost::program_options::executable>
      : ostream_formatter
  {};
}

#endif
