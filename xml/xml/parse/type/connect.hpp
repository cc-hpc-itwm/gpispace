// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_CONNECT_HPP
#define _XML_PARSE_TYPE_CONNECT_HPP

#include <xml/parse/type/connect.fwd.hpp>

#include <xml/parse/id/generic.hpp>

#include <xml/parse/type/transition.fwd.hpp>

#include <fhg/util/xml.fwd.hpp>

#include <we/net.hpp>
#include <we/type/property.hpp>

#include <string>

#include <boost/optional.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct connect_type
      {
        ID_SIGNATURES(connect);
        PARENT_SIGNATURES(transition);

      public:
        typedef std::pair<std::string, std::string> unique_key_type;

        connect_type ( ID_CONS_PARAM(connect)
                     , PARENT_CONS_PARAM(transition)
                     , const std::string& place
                     , const std::string& port
                     , const ::petri_net::edge::type& direction
                     , const we::type::property::type& properties
                     = we::type::property::type()
                     );

        const std::string& place() const;
        const std::string& port() const;
        boost::optional<const id::ref::place&> resolved_place() const;
        boost::optional<const id::ref::port&> resolved_port() const;

        const ::petri_net::edge::type& direction() const;
        const ::petri_net::edge::type& direction
          (const ::petri_net::edge::type&);

      private:
        friend struct net_type;
        const std::string& place (const std::string&);

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

        ::petri_net::edge::type _direction;

        we::type::property::type _properties;
      };

      namespace dump
      {
        void dump ( ::fhg::util::xml::xmlstream & s
                  , const connect_type & c
                  , const std::string & type
                  );
      }
    }
  }
}

#endif
