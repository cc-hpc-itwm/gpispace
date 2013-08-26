// mirko.rahn@itwm.fraunhofer.de

#include <fhg/util/cpp/namespace.hpp>
#include <fhg/util/cpp/block.hpp>

#include <iostream>

namespace fhg
{
  namespace util
  {
    namespace cpp
    {
      namespace ns
      {
        open::open ( fhg::util::indenter& indent
                   , const boost::optional<std::string>& tag
                   )
          : _indent (indent)
          , _tag (tag)
        {}

        open::open (fhg::util::indenter& indent, const std::string& tag)
          : _indent (indent)
          , _tag (tag)
        {}
        std::ostream& open::operator() (std::ostream& os) const
        {
          os << _indent << "namespace";
          if (_tag)
          {
            os << " " << *_tag;
          }
          return os << block::open (_indent);
        }

        close::close (fhg::util::indenter& indent)
          : _indent (indent)
        {}
        std::ostream& close::operator() (std::ostream& os) const
        {
          return os << block::close (_indent);
        }
      }
    }
  }
}
