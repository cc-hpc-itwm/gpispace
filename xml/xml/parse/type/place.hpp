// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_PLACE_HPP
#define _XML_PARSE_TYPE_PLACE_HPP

#include <xml/parse/state.fwd.hpp>
#include <xml/parse/type/token.hpp>
#include <xml/parse/type_map_type.hpp>
#include <xml/parse/util/id_type.hpp>
#include <xml/parse/util/unique.hpp>

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
      typedef std::list<value::type> values_type;
      typedef std::list<token_type> tokens_type;

      struct place_type
      {
      private:
        fhg::util::maybe<bool> _is_virtual;

        id::place _id;
        id::net _parent;

        std::string _name;

      public:
        const std::string& name() const;
        const std::string& name(const std::string& name);
        std::string type;
        tokens_type tokens;
        values_type values;
        signature::type sig;
        we::type::property::type prop;

        place_type ( const std::string & name
                   , const std::string & _type
                   , const fhg::util::maybe<bool> is_virtual
                   , const id::place& id
                   , const id::net& parent
                   );

        place_type (const id::place& id, const id::net& parent);

        const id::place& id() const;
        const id::net& parent() const;

        bool is_same (const place_type& other) const;

        void push_token (const token_type & t);

        void translate ( const boost::filesystem::path & path
                       , const state::type & state
                       );

        void specialize ( const type::type_map_type & map_in
                        , const state::type &
                        );

        const fhg::util::maybe<bool>& get_is_virtual (void) const;
        bool is_virtual (void) const;
      };

      typedef xml::util::uniquePP<place_type>::elements_type places_type;

      namespace dump
      {
        void dump (::fhg::util::xml::xmlstream & s, const place_type & p);
      }
    }
  }
}

#endif
