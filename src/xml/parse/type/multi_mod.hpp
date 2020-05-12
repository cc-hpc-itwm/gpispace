#pragma once

#include <unordered_map>

#include <xml/parse/type/mod.hpp>
#include <xml/parse/type/preferences.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      using multi_module_map = std::unordered_map < preference_type
                                                  , module_type
                                                  >;

      struct multi_module_type
      {
      public:
        multi_module_type () = default;

        void add (module_type const& mod);

        multi_module_map const& modules() const;

        boost::optional<we::type::eureka_id_type> const& eureka_id() const;

      private:
        multi_module_map  _modules;
      };

      namespace dump
      {
        void dump (::fhg::util::xml::xmlstream&, const multi_module_type&);
      }
    }
  }
}
