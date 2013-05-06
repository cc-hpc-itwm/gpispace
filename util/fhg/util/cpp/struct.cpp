// mirko.rahn@itwm.fraunhofer.de

#include <fhg/util/cpp/struct.hpp>
#include <fhg/util/cpp/block.hpp>

#include <iostream>

namespace fhg
{
  namespace util
  {
    namespace cpp
    {
      namespace structure
      {
        open::open (fhg::util::indenter& indent)
          : _indent (indent)
          , _tag (boost::none)
        {}
        open::open ( fhg::util::indenter& indent
                   , const std::string& tag
                   )
          : _indent (indent)
          , _tag (tag)
        {}
        std::ostream& open::operator() (std::ostream& os) const
        {
          os << _indent << "struct";
          if (_tag)
          {
            os << " " << *_tag;
          }
          return os << block::open (_indent);
        }
        std::ostream& operator<< (std::ostream& os, const open& b)
        {
          return b (os);
        }

        close::close (fhg::util::indenter& indent)
          : _indent (indent)
        {}
        std::ostream& close::operator() (std::ostream& os) const
        {
          return os << block::close (_indent) << ";";
        }
        std::ostream& operator<< (std::ostream& os, const close& b)
        {
          return b (os);
        }
      }
    }
  }
}
