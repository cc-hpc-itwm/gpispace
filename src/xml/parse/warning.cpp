// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <xml/parse/warning.hpp>

#include <xml/parse/type/connect.hpp>
#include <xml/parse/type/function.hpp>
#include <xml/parse/type/place.hpp>
#include <xml/parse/type/place_map.hpp>
#include <xml/parse/type/port.hpp>
#include <xml/parse/type/template.hpp>
#include <xml/parse/type/transition.hpp>

#include <we/type/net.hpp>

#include <boost/format.hpp>

namespace xml
{
  namespace parse
  {
    namespace warning
    {
      port_not_connected::port_not_connected
        (type::port_type const& port, ::boost::filesystem::path const& path)
          : generic ( ::boost::format ("%1%-port %2% not connected in %3%")
                    % port.direction()
                    % port.name()
                    % path
                    )
          , _path (path)
      { }

      conflicting_port_types::conflicting_port_types
        ( type::transition_type const& transition
        , type::port_type const& in
        , type::port_type const& out
        , ::boost::filesystem::path const& path
        )
          : generic ( ::boost::format ( "port %1% of transition %2% has differing "
                                      "types for input (%3%) and output (%4%) "
                                      "in %5%"
                                    )
                    % in.name()
                    % transition.name()
                    % in.type()
                    % out.type()
                    % path
                    )
          , _path (path)
      { }

      overwrite_function_name_trans::overwrite_function_name_trans
        (type::transition_type const& trans, type::function_type const& function)
          : generic ( ::boost::format ( "name of function %1% defined at %2% "
                                      "overwritten with name of transition %3% "
                                      "at %4%"
                                    )
                    % function.name().get_value_or ("<<anonymous>>")
                    % function.position_of_definition()
                    % trans.name()
                    % trans.position_of_definition()
                    )
      { }

      duplicate_external_function::duplicate_external_function
        (type::module_type const& mod, type::module_type const& old)
          : generic ( ::boost::format ( "the external function %1% in module %2%"
                                      " has multiple occurences in %3% and %4%"
                                    )
                    % mod.function()
                    % mod.name()
                    % old.position_of_definition()
                    % mod.position_of_definition()
                    )
      {}

      synthesize_anonymous_function::synthesize_anonymous_function
        (type::function_type const& function)
          : generic ( ::boost::format ( "synthesize anonymous top level function"
                                      " at %1%"
                                    )
                    % function.position_of_definition()
                    )
      {}

      struct_redefined::struct_redefined ( type::structure_type const& early
                                         , type::structure_type const& late
                                         )
        : generic ( ::boost::format ("struct %1% at %2% redefined at %3%")
                  % early.name()
                  % early.position_of_definition()
                  % late.position_of_definition()
                  )
      {}
    }
  }
}
