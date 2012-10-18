// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_SPECIALIZE_HPP
#define _XML_PARSE_TYPE_SPECIALIZE_HPP

#include <xml/parse/state.fwd.hpp>
#include <xml/parse/type/struct.hpp>
#include <xml/parse/type_map_type.hpp>
#include <xml/parse/util/id_type.hpp>

#include <fhg/util/xml.fwd.hpp>

#include <string>

#include <boost/filesystem.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct specialize_type
      {
      public:
        specialize_type (const id::specialize& id, const id::net& parent);

        const id::specialize& id() const;
        const id::net& parent() const;
        bool is_same (const specialize_type& other) const;

      public:
        std::string name;
        std::string use;
        type_map_type type_map;
        type_get_type type_get;
        boost::filesystem::path path;

      private:
        id::specialize _id;
        id::net _parent;
      };

      void split_structs ( const parse::struct_t::set_type & global
                         , structs_type & child_structs
                         , structs_type & parent_structs
                         , const type_get_type & type_get
                         , const state::type & state
                         );

      void specialize_structs ( const type_map_type & map
                              , structs_type & structs
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
