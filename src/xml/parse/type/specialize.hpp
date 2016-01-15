// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#pragma once

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
      public:
        typedef std::string unique_key_type;

        specialize_type ( const util::position_type&
                        , const std::string& name
                        , const std::string& use
                        , const type_map_type& type_map
                        , const type_get_type& type_get
                        );

        const std::string& name () const;
        const unique_key_type& unique_key() const;

      private:
        std::string _name;

        //! \todo All these should be private wth accessors.
      public:
        std::string use;
        type_map_type type_map;
        type_get_type type_get;
      };

      void split_structs ( const parse::structure_type_util::set_type & global
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
