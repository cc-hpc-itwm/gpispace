// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_SPECIALIZE_HPP
#define _XML_PARSE_TYPE_SPECIALIZE_HPP

#include <xml/parse/id/generic.hpp>
#include <xml/parse/state.fwd.hpp>
#include <xml/parse/type/struct.hpp>
#include <xml/parse/type/net.fwd.hpp>
#include <xml/parse/type_map_type.hpp>
#include <xml/parse/type/with_position_of_definition.hpp>
#include <xml/parse/util/position.fwd.hpp>

#include <fhg/util/xml.fwd.hpp>

#include <string>

#include <boost/filesystem.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct specialize_type : with_position_of_definition
      {
        ID_SIGNATURES(specialize);
        PARENT_SIGNATURES(net);

      public:
        typedef std::string unique_key_type;

        specialize_type ( ID_CONS_PARAM(specialize)
                        , PARENT_CONS_PARAM(net)
                        , const util::position_type&
                        , const std::string& name
                        , const std::string& use
                        , const type_map_type& type_map
                        , const type_get_type& type_get
                        );

        const std::string& name () const;
        const std::string& name (const std::string& name);

      private:
        friend struct net_type;
        const std::string& name_impl (const std::string& name);

      public:
        const unique_key_type& unique_key() const;

        id::ref::specialize clone
          ( const boost::optional<parent_id_type>& parent = boost::none
          , const boost::optional<id::mapper*>& mapper = boost::none
          ) const;

      private:
        std::string _name;

        //! \todo All these should be private wth accessors.
      public:
        std::string use;
        type_map_type type_map;
        type_get_type type_get;
      };

      void split_structs ( const parse::structure_type::set_type & global
                         , structs_type & child_structs
                         , structs_type & parent_structs
                         , const type_get_type & type_get
                         , const state::type & state
                         );

      namespace dump
      {
        void dump ( ::fhg::util::xml::xmlstream & s
                  , const specialize_type & sp
                  );
      }
    }
  }
}

#endif
