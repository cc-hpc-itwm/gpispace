// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_PLACE_HPP
#define _XML_PARSE_TYPE_PLACE_HPP

#include <xml/parse/id/generic.hpp>
#include <xml/parse/state.fwd.hpp>
#include <xml/parse/type/token.hpp>
#include <xml/parse/type_map_type.hpp>
#include <xml/parse/type/net.fwd.hpp>

#include <fhg/util/maybe.hpp>
#include <fhg/util/xml.fwd.hpp>

#include <we/type/property.hpp>
#include <we/type/signature.hpp>
#include <we/type/value.hpp>

#include <string>
#include <list>

#include <boost/filesystem.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct place_type
      {
        ID_SIGNATURES(place);
        PARENT_SIGNATURES(net);

      public:
        typedef std::list<value::type> values_type;

        place_type ( ID_CONS_PARAM(place)
                   , PARENT_CONS_PARAM(net)
                   );

        place_type ( ID_CONS_PARAM(place)
                   , PARENT_CONS_PARAM(net)
                   , const std::string & name
                   , const std::string & _type
                   , const fhg::util::maybe<bool> is_virtual
                   );

        place_type ( ID_CONS_PARAM(place)
                   , PARENT_CONS_PARAM(net)
                   , const fhg::util::maybe<bool>& _is_virtual
                   , const std::string& name
                   , const std::string& type
                   , const std::list<token_type>& tokens
                   , const values_type& values
                   , const signature::type& sig
                   , const we::type::property::type& prop
                   );

        const std::string& name() const;
        const std::string& name(const std::string& name);


        void push_token (const token_type & t);

        void translate ( const boost::filesystem::path & path
                       , const state::type & state
                       );

        void specialize ( const type::type_map_type & map_in
                        , const state::type &
                        );

        const fhg::util::maybe<bool>& get_is_virtual (void) const;
        bool is_virtual (void) const;

        id::ref::place clone() const;

      private:
        fhg::util::maybe<bool> _is_virtual;

        std::string _name;

        //! \todo All these should be private with accessors.
      public:
        std::string type;
        std::list<token_type> tokens;
        values_type values;
        signature::type sig;
        we::type::property::type prop;
      };

      namespace dump
      {
        void dump (::fhg::util::xml::xmlstream & s, const place_type & p);
      }
    }
  }
}

#endif
