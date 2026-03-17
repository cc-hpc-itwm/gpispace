#ifndef FHG_UTIL_BOOST_PROGRAM_OPTIONS_VALIDATORS_NONEMPTY_STRING_HPP
#define FHG_UTIL_BOOST_PROGRAM_OPTIONS_VALIDATORS_NONEMPTY_STRING_HPP

#include <gspc/util/boost/program_options/validators.hpp>




      namespace gspc::util::boost::program_options
      {
        struct nonempty_string : public std::string
        {
          nonempty_string (std::string const& option)
            : std::string (option)
          {
            if (this->empty())
            {
              throw ::boost::program_options::invalid_option_value
                ("empty string");
            }
          }
        };

        inline void validate ( ::boost::any& v
                             , const std::vector<std::string>& values
                             , nonempty_string*
                             , int
                             )
        {
          validate<nonempty_string> (v, values);
        }
      }




#endif
