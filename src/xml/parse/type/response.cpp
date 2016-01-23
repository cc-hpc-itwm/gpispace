#include <xml/parse/type/response.hpp>

#include <xml/parse/type/transition.hpp>
#include <xml/parse/util/position.hpp>

#include <fhg/util/xml.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      response_type::response_type ( util::position_type const& pod
                                   , std::string const& port
                                   , std::string const& to
                                   , we::type::property::type const& properties
                                   )
        : with_position_of_definition (pod)
        , _port (port)
        , _to (to)
        , _properties (properties)
      {}

      namespace dump
      {
        void dump (::fhg::util::xml::xmlstream& s, response_type const& r)
        {
          s.open ("connect-response");
          s.attr ("port", r.port());
          s.attr ("to", r.to());

          ::we::type::property::dump::dump (s, r.properties());

          s.close();
        }
      }
    }
  }
}
