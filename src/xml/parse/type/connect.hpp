// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_CONNECT_HPP
#define _XML_PARSE_TYPE_CONNECT_HPP

#include <xml/parse/type/connect.fwd.hpp>

#include <xml/parse/id/generic.hpp>
#include <xml/parse/type/transition.fwd.hpp>
#include <xml/parse/type/with_position_of_definition.hpp>
#include <xml/parse/util/position.fwd.hpp>

#include <fhg/util/std/tuple.hpp> //! \note To allow storing in unique.
#include <fhg/util/xml.fwd.hpp>

#include <we/type/net.hpp>
#include <we/type/property.hpp>

#include <string>
#include <tuple>

#include <boost/optional.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct connect_type : with_position_of_definition
      {
        ID_SIGNATURES(connect);
        PARENT_SIGNATURES(transition);

      public:
        //! \note          place,       port,        PT||PT_READ
        typedef std::tuple<std::string, std::string, bool> unique_key_type;

        connect_type ( ID_CONS_PARAM(connect)
                     , PARENT_CONS_PARAM(transition)
                     , const util::position_type&
                     , const std::string& place
                     , const std::string& port
                     , const ::we::edge::type& direction
                     , const we::type::property::type& properties
                     = we::type::property::type()
                     );

        const std::string& place() const;
        const std::string& port() const;
        boost::optional<const id::ref::place&> resolved_place() const;
        boost::optional<const id::ref::port&> resolved_port() const;

        const ::we::edge::type& direction() const;
        const ::we::edge::type& direction
          (const ::we::edge::type&);

        const std::string& place (const std::string&);

      private:
        friend struct transition_type;
        const ::we::edge::type& direction_impl
          (const ::we::edge::type&);

        const std::string& place_impl (const std::string&);

      public:
        const we::type::property::type& properties() const;
        we::type::property::type& properties();

        unique_key_type unique_key() const;

        id::ref::connect clone
          ( const boost::optional<parent_id_type>& parent = boost::none
          , const boost::optional<id::mapper*>& mapper = boost::none
          ) const;

      private:
        //! \todo Should be a id::place and id::port.
        //! \note In principle yes but we do have connections to
        //! not yet parsed places
        std::string _place;
        std::string _port;

        ::we::edge::type _direction;

        we::type::property::type _properties;
      };

      namespace dump
      {
        void dump (::fhg::util::xml::xmlstream&, const connect_type&);
      }
    }
  }
}

#endif
