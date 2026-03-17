#ifndef FHG_UTIL_BOOST_PROGRAM_OPTIONS_VALIDATORS_NONEXISTING_PATH_HPP
#define FHG_UTIL_BOOST_PROGRAM_OPTIONS_VALIDATORS_NONEXISTING_PATH_HPP

#include <gspc/util/boost/program_options/validators.hpp>

#include <filesystem>
#include <gspc/util/fmt/std/filesystem/path.formatter.hpp>
#include <fmt/core.h>




      namespace gspc::util::boost::program_options
      {
        struct nonexisting_path
        {
          nonexisting_path (std::string const& option)
            : nonexisting_path {std::filesystem::path {option}}
          {}

          nonexisting_path (std::filesystem::path const& path)
            : _path {path}
          {
            if (std::filesystem::exists (_path))
            {
              throw ::boost::program_options::invalid_option_value
                { fmt::format ("{} already exists.", _path)
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
                             , nonexisting_path*
                             , int
                             )
        {
          validate<nonexisting_path> (v, values);
        }

        inline auto operator<<
          ( std::ostream& os
          , nonexisting_path const& p
          ) -> std::ostream&
        {
          return os << static_cast<std::filesystem::path> (p);
        }
      }




#endif
