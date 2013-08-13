// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_SIGNATURE_CPP_HPP
#define _WE_TYPE_SIGNATURE_CPP_HPP

#include <we/type/signature.hpp>

#include <we/type/literal/cpp.hpp>

#include <fhg/util/cpp/include.hpp>
#include <fhg/util/cpp/include_guard.hpp>

namespace cpp_util = fhg::util::cpp;

#include <iostream>
#include <string>

#include <boost/filesystem.hpp>
#include <boost/unordered_set.hpp>

#include <we2/type/signature/cpp.hpp>
#include <we2/type/compat.sig.hpp>

namespace signature
{
  namespace cpp
  {
    namespace visitor
    {
      typedef boost::unordered_set<std::string> seen_type;

      template<typename Stream>
      class cpp_includes : public boost::static_visitor<void>
      {
      private:
        Stream& os;
        seen_type & seen;

      public:
        cpp_includes ( Stream& _os
                     , seen_type & _seen
                     )
          : os (_os), seen (_seen)
        {}

        void operator () (const std::string & t) const
        {
          if (!literal::cpp::known (t))
            {
              if (seen.find (t) == seen.end())
                {
                  os << cpp_util::include ("pnetc/type/" + t + ".hpp");

                  seen.insert (t);
                }
            }
        }

        void operator () (const structured_t & map) const
        {
          for ( structured_t::const_iterator field (map.begin())
              ; field != map.end()
              ; ++field
              )
            {
              boost::apply_visitor ( cpp_includes (os, seen)
                                   , field->second
                                   );
            }
        }
      };
    }

    template<typename Stream>
      void cpp_header
      ( Stream& os
      , const type & s
      , const std::string & n
      , const boost::filesystem::path & defpath
      )
    {
      os << cpp_util::include_guard::open ("PNETC_TYPE_" + n);

      visitor::seen_type seen;

      boost::apply_visitor ( visitor::cpp_includes<Stream> (os, seen)
                           , s.desc()
                           );

      os << pnet::type::signature::cpp::header_signature
        (pnet::type::compat::COMPAT (s, n))
         << std::endl;

      os << cpp_util::include_guard::close();
    }

    template<typename Stream>
      void cpp_implementation
      ( Stream& os
      , const type & s
      , const std::string & n
      , const boost::filesystem::path & defpath
      )
    {
      os << "// defined in " << defpath << std::endl;

      os << cpp_util::include ("pnetc/type/" + n + ".hpp");

      os << pnet::type::signature::cpp::impl_signature
        (pnet::type::compat::COMPAT (s, n))
         << std::endl;
    }
  }
}

#endif
