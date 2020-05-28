#include <xml/parse/type/memory_transfer.hpp>
#include <xml/parse/util/position.hpp>

#include <fhg/util/xml.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      memory_transfer_type::memory_transfer_type
        ( const util::position_type& position_of_definition
        , std::string const& global
        , std::string const& local
        , const we::type::property::type& properties
        )
          : with_position_of_definition (position_of_definition)
          , _global (global)
          , _local (local)
          , _properties (properties)
      {}

      memory_get::memory_get
        ( const util::position_type& position_of_definition
        , std::string const& global
        , std::string const& local
        , const we::type::property::type& properties
        )
          : memory_transfer_type
            (position_of_definition, global, local, properties)
      {}

      memory_put::memory_put
        ( const util::position_type& position_of_definition
        , std::string const& global
        , std::string const& local
        , const we::type::property::type& properties
        , boost::optional<bool> const& not_modified_in_module_call
        )
          : memory_transfer_type
            (position_of_definition, global, local, properties)
          , _not_modified_in_module_call (not_modified_in_module_call)
      {}

      memory_getput::memory_getput
        ( const util::position_type& position_of_definition
        , std::string const& global
        , std::string const& local
        , const we::type::property::type& properties
        , boost::optional<bool> const& not_modified_in_module_call
        )
          : memory_transfer_type
            (position_of_definition, global, local, properties)
          , _not_modified_in_module_call (not_modified_in_module_call)
      {}

      namespace dump
      {
        namespace
        {
          void dump_transfer ( ::fhg::util::xml::xmlstream& s
                             , memory_transfer_type const& mt
                             )
          {
            s.open ("global");
            s.content (mt.global());
            s.close();
            s.open ("local");
            s.content (mt.local());
            s.close();
          }
        }

        void dump (::fhg::util::xml::xmlstream& s, const memory_get& mg)
        {
          s.open ("memory-get");
          dump_transfer (s, mg);
          s.close();
        }
        void dump (::fhg::util::xml::xmlstream& s, const memory_put& mp)
        {
          s.open ("memory-put");
          s.attr
            ("not-modified-in-module-call", mp.not_modified_in_module_call());
          dump_transfer (s, mp);
          s.close();
        }
        void dump (::fhg::util::xml::xmlstream& s, const memory_getput& mp)
        {
          s.open ("memory-getput");
          s.attr
            ("not-modified-in-module-call", mp.not_modified_in_module_call());
          dump_transfer (s, mp);
          s.close();
        }
      }
    }
  }
}
