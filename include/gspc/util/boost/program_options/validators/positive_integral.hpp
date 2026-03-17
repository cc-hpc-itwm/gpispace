#ifndef FHG_UTIL_BOOST_PROGRAM_OPTIONS_VALIDATORS_POSITIVE_INTEGRAL_HPP
#define FHG_UTIL_BOOST_PROGRAM_OPTIONS_VALIDATORS_POSITIVE_INTEGRAL_HPP

#include <gspc/util/boost/program_options/validators/integral_greater_than.hpp>




      namespace gspc::util::boost::program_options
      {
        template<typename Base>
          using positive_integral = integral_greater_than<Base, 0>;
      }




#endif
