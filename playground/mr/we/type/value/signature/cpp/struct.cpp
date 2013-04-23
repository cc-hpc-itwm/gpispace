// mirko.rahn@itwm.fraunhofer.de

#include <we/type/value/signature/cpp/struct.hpp>

#include <we/type/value/signature/name_of.hpp>

#include <fhg/util/cpp/block.hpp>

#include <boost/foreach.hpp>
#include <boost/optional.hpp>

#include <iostream>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      as_struct::as_struct ( const signature_type& signature
                           , fhg::util::indenter& indent
                           )
        : _signature (signature)
        , _indent (indent)
      {}

      namespace
      {
        class visitor_struct : public boost::static_visitor<std::ostream&>
        {
        public:
          visitor_struct (std::ostream& os, fhg::util::indenter& indent)
            : _os (os)
            , _indent (indent)
          {}

          std::ostream& operator() (const structured_type& m) const
          {
            namespace block = fhg::util::cpp::block;

            _os << block::open (_indent, "struct") << std::endl;
            BOOST_FOREACH (const structured_type::value_type& nv, m)
            {
              boost::apply_visitor (*this, nv.second);
              _os << " " << nv.first << ";" << std::endl;
            }
            return _os << block::close (_indent);
          }

          template<typename T>
          std::ostream& operator() (const T& x) const
          {
            return _os << _indent << name_of (x);
          }

        private:
          std::ostream& _os;
          fhg::util::indenter& _indent;
        };
      }

      std::ostream& as_struct::operator() (std::ostream& os) const
      {
        return boost::apply_visitor ( visitor_struct (os, _indent)
                                    , _signature.value()
                                    );
      }

      std::ostream& operator<< (std::ostream& os, const as_struct& as)
      {
        return as (os);
      }
    }
  }
}
