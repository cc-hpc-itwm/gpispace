#ifndef FHG_UTIL_BOOST_PROGRAM_OPTIONS_VALIDATORS_EXISTING_PATH_HPP
#define FHG_UTIL_BOOST_PROGRAM_OPTIONS_VALIDATORS_EXISTING_PATH_HPP

#include <gspc/util/boost/program_options/validators.hpp>

#include <filesystem>
#include <gspc/util/fmt/std/filesystem/path.formatter.hpp>
#include <fmt/core.h>




      namespace gspc::util::boost::program_options
      {
        struct existing_path
        {
          existing_path (std::string const& option)
            : existing_path {std::filesystem::path {option}}
          {}

          existing_path (std::filesystem::path const& path)
            : _path {std::filesystem::canonical (path)}
          {
            if (!std::filesystem::exists (_path))
            {
              throw ::boost::program_options::invalid_option_value
                { fmt::format ("{} does not exist.", _path)
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
                             , existing_path*
                             , int
                             )
        {
          validate<existing_path> (v, values);
        }

        inline auto operator<<
          ( std::ostream& os
          , existing_path const& p
          ) -> std::ostream&
        {
          return os << static_cast<std::filesystem::path> (p);
        }
      }




#endif
