#ifndef FHG_UTIL_BOOST_PROGRAM_OPTIONS_VALIDATORS_NONEMPTY_FILE_HPP
#define FHG_UTIL_BOOST_PROGRAM_OPTIONS_VALIDATORS_NONEMPTY_FILE_HPP

#include <gspc/util/boost/program_options/validators.hpp>

#include <gspc/util/boost/program_options/validators/existing_path.hpp>

#include <filesystem>
#include <gspc/util/fmt/std/filesystem/path.formatter.hpp>
#include <fmt/core.h>




      namespace gspc::util::boost::program_options
      {
        struct nonempty_file
        {
          nonempty_file (std::string const& option)
            : nonempty_file {std::filesystem::path {option}}
          {}

          nonempty_file (std::filesystem::path const& path)
            : _path {existing_path {path}}
          {
            if (! (std::filesystem::file_size (_path) > 0))
            {
              throw ::boost::program_options::invalid_option_value
                { fmt::format ("{} has size zero.", _path)
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
                             , nonempty_file*
                             , int
                             )
        {
          validate<nonempty_file> (v, values);
        }

        inline auto operator<<
          ( std::ostream& os
          , nonempty_file const& p
          ) -> std::ostream&
        {
          return os << static_cast<std::filesystem::path> (p);
        }
      }




#endif
