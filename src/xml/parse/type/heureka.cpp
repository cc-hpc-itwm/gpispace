#include <xml/parse/type/heureka.hpp>

#include <xml/parse/type/transition.hpp>
#include <xml/parse/util/position.hpp>

#include <fhg/util/xml.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      heureka_type::heureka_type ( util::position_type const& pod
                                 , std::string const& port
                                 )
        : with_position_of_definition (pod)
        , _port (port)
      {}

      namespace dump
      {
        void dump (::fhg::util::xml::xmlstream& s, heureka_type const& r)
        {
          s.open ("connect-heureka");
          s.attr ("port", r.port());

          s.close();
        }
      }
    }
  }
}
