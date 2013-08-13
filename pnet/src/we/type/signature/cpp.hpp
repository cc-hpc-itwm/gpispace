// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_SIGNATURE_CPP_HPP
#define _WE_TYPE_SIGNATURE_CPP_HPP

#include <fhg/util/cpp/include.hpp>
#include <fhg/util/cpp/include_guard.hpp>

#include <iostream>
#include <string>

#include <boost/filesystem.hpp>
#include <boost/unordered_set.hpp>

#include <we2/type/signature/cpp.hpp>
#include <we2/type/signature/names.hpp>
#include <we2/type/signature/is_literal.hpp>
#include <we2/type/compat.sig.hpp>

namespace signature
{
  namespace cpp
  {
    template<typename Stream>
      void cpp_header
      ( Stream& os
      , const type & s
      , const std::string & n
      , const boost::filesystem::path & defpath
      )
    {
      const pnet::type::signature::signature_type sig
        (pnet::type::compat::COMPAT (s, n));

      os << fhg::util::cpp::include_guard::open ("PNETC_TYPE_" + n);

      const boost::unordered_set<std::string> names
        (pnet::type::signature::names (sig));

      BOOST_FOREACH (const std::string& tname, names)
      {
        if (!pnet::type::signature::is_literal (tname))
        {
          os << fhg::util::cpp::include ("pnetc/type/" + tname + ".hpp");
        }
      }

      os << pnet::type::signature::cpp::header_signature (sig) << std::endl;

      os << fhg::util::cpp::include_guard::close();
    }

    template<typename Stream>
      void cpp_implementation
      ( Stream& os
      , const type & s
      , const std::string & n
      , const boost::filesystem::path & defpath
      )
    {
      const pnet::type::signature::signature_type sig
        (pnet::type::compat::COMPAT (s, n));

      os << "// defined in " << defpath << std::endl;

      os << fhg::util::cpp::include ("pnetc/type/" + n + ".hpp");

      os << pnet::type::signature::cpp::impl_signature (sig) << std::endl;
    }
  }
}

#endif
