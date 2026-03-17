#ifndef FHG_UTIL_BOOST_PROGRAM_OPTIONS_VALIDATORS_IS_DIRECTORY_IF_EXISTS_HPP
#define FHG_UTIL_BOOST_PROGRAM_OPTIONS_VALIDATORS_IS_DIRECTORY_IF_EXISTS_HPP

#include <gspc/util/boost/program_options/validators.hpp>

#include <filesystem>
#include <gspc/util/fmt/std/filesystem/path.formatter.hpp>
#include <fmt/core.h>




      namespace gspc::util::boost::program_options
      {
        struct is_directory_if_exists
        {
          is_directory_if_exists (std::string const& option)
            : is_directory_if_exists {std::filesystem::path {option}}
          {}

          is_directory_if_exists (std::filesystem::path const& path)
            : _path {path}
          {
            if ( std::filesystem::exists (_path)
               && !std::filesystem::is_directory (_path)
               )
            {
              throw ::boost::program_options::invalid_option_value
                { fmt::format ("{} exists and is not a directory.", path)
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
                             , is_directory_if_exists*
                             , int
                             )
        {
          validate<is_directory_if_exists> (v, values);
        }

        inline auto operator<<
          ( std::ostream& os
          , is_directory_if_exists const& p
          ) -> std::ostream&
        {
          return os << static_cast<std::filesystem::path> (p);
        }
      }




#endif
