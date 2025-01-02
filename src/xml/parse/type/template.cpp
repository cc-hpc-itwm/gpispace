// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <xml/parse/type/template.hpp>

#include <xml/parse/type/net.hpp>

#include <fhg/util/xml.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      tmpl_type::tmpl_type
        ( util::position_type const& pod
        , ::boost::optional<std::string> const& name
        , names_type const& tmpl_parameter
        , function_type const& function
        )
          : with_position_of_definition (pod)
          , _name (name)
          , _tmpl_parameter (tmpl_parameter)
          , _function (function)
      {}

      ::boost::optional<std::string> const& tmpl_type::name() const
      {
        return _name;
      }

      tmpl_type::names_type const&
      tmpl_type::tmpl_parameter () const
      {
        return _tmpl_parameter;
      }

      function_type const& tmpl_type::function() const
      {
        return _function;
      }

      void tmpl_type::resolve_function_use_recursive
        (std::unordered_map<std::string, function_type const&> known)
      {
        _function.resolve_function_use_recursive (known);
      }

      void tmpl_type::resolve_types_recursive
        (std::unordered_map<std::string, pnet::type::signature::signature_type> known)
      {
        _function.resolve_types_recursive (known);
      }

      tmpl_type::unique_key_type const& tmpl_type::unique_key() const
      {
        //! \note Anonymous templates can't be stored in unique, thus
        //! just indirect.
        return *name();
      }

      namespace dump
      {
        void dump (fhg::util::xml::xmlstream & s, tmpl_type const& t)
        {
          s.open ("template");
          s.attr ("name", t.name());

          for (std::string const& tn : t.tmpl_parameter())
            {
              s.open ("template-parameter");
              s.attr ("type", tn);
              s.close ();
            }

          ::xml::parse::type::dump::dump (s, t.function());

          s.close ();
        }
      }
    }
  }
}
