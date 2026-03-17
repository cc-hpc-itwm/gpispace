#ifndef FHG_UTIL_BOOST_PROGRAM_OPTIONS_VALIDATORS_EXISTING_DIRECTORY_HPP
#define FHG_UTIL_BOOST_PROGRAM_OPTIONS_VALIDATORS_EXISTING_DIRECTORY_HPP

#include <gspc/util/boost/program_options/validators/existing_path.hpp>

#include <gspc/util/boost/program_options/validators.hpp>

#include <filesystem>
#include <gspc/util/fmt/std/filesystem/path.formatter.hpp>
#include <fmt/core.h>




      namespace gspc::util::boost::program_options
      {
        struct existing_directory
        {
          existing_directory (std::string const& option)
            : existing_directory {std::filesystem::path {option}}
          {}

          existing_directory (std::filesystem::path const& path)
            : _path {existing_path {path}}
          {
            if (!std::filesystem::is_directory (_path))
            {
              throw ::boost::program_options::invalid_option_value
                { fmt::format ( "{} is not a directory.", _path)
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
                             , existing_directory*
                             , int
                             )
        {
          validate<existing_directory> (v, values);
        }

        inline auto operator<<
          ( std::ostream& os
          , existing_directory const& p
          ) -> std::ostream&
        {
          return os << static_cast<std::filesystem::path> (p);
        }
      }




#endif
