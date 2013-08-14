// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_PLACE_HPP
#define _XML_PARSE_TYPE_PLACE_HPP

#include <xml/parse/id/generic.hpp>
#include <xml/parse/state.fwd.hpp>
#include <xml/parse/type_map_type.hpp>
#include <xml/parse/type/net.fwd.hpp>
#include <xml/parse/type/with_position_of_definition.hpp>
#include <xml/parse/util/position.fwd.hpp>

#include <fhg/util/xml.fwd.hpp>

#include <we/type/property.hpp>
#include <we/type/signature.hpp>

#include <string>
#include <list>

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct place_type : with_position_of_definition
      {
        ID_SIGNATURES(place);
        PARENT_SIGNATURES(net);

      public:
        typedef std::string unique_key_type;

        typedef std::string token_type;

        place_type ( ID_CONS_PARAM(place)
                   , PARENT_CONS_PARAM(net)
                   , const util::position_type&
                   , const std::string & name
                   , const std::string & type
                   , const boost::optional<bool> is_virtual
                   );

        place_type ( ID_CONS_PARAM(place)
                   , PARENT_CONS_PARAM(net)
                   , const util::position_type&
                   , const boost::optional<bool>& _is_virtual
                   , const std::string& name
                   , const std::string& type
                   , const std::list<token_type>& tokens
                   , const we::type::property::type& properties
                   );

        const std::string& name() const;
        const std::string& name (const std::string& name);

        const std::string& type() const;
        const std::string& type (const std::string&);

        boost::optional<signature::type> signature() const;
        signature::type signature_or_throw() const;

      private:
        friend struct net_type;
        const std::string& name_impl (const std::string& name);

      public:
        void push_token (const token_type & t);

        void specialize ( const type::type_map_type & map_in
                        , const state::type &
                        );

        const boost::optional<bool>& get_is_virtual (void) const;
        bool is_virtual (void) const;
        void set_virtual (bool);

        const we::type::property::type& properties() const;
        we::type::property::type& properties();

        const unique_key_type& unique_key() const;

        id::ref::place clone
          ( const boost::optional<parent_id_type>& parent = boost::none
          , const boost::optional<id::mapper*>& mapper = boost::none
          ) const;

      private:
        boost::optional<bool> _is_virtual;

        std::string _name;
        std::string _type;

        //! \todo All these should be private with accessors.
      public:
        std::list<token_type> tokens;

      private:
        we::type::property::type _properties;
      };

      namespace dump
      {
        void dump (::fhg::util::xml::xmlstream & s, const place_type & p);
      }
    }
  }
}

#endif
