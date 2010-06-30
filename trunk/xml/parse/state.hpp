// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_STATE_HPP
#define _XML_PARSE_STATE_HPP

#include <we/type/signature.hpp>

#include <iostream>

// ************************************************************************* //

namespace xml
{
  namespace parse
  {
    namespace state
    {
      struct type
      {
      private:
        signature::set_type _signature;
        int _level;

      public:
        type (void)
          : _signature ()
          , _level (0)
        {}

        int & level (void) { return _level; }
        signature::set_type & signature (void) { return _signature; }

        void resolve_signatures (void)
        {
          signature::visitor::resolve resolve (_signature);

          for ( signature::set_type::iterator sig (_signature.begin())
              ; sig != _signature.end()
              ; ++sig
              )
            {
              boost::apply_visitor (resolve, sig->second);
            }
        }

        void print_signatures (std::ostream & s) const
        {
          s << "signatures:" << std::endl;

          for ( signature::set_type::const_iterator pos (_signature.begin())
              ; pos != _signature.end()
              ; ++pos
              )
            {
              s << pos->first << ": " << pos->second << std::endl;
            }
        }
      };
    }
  }
}

#endif
