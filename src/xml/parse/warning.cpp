// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <xml/parse/warning.hpp>

#include <xml/parse/type/connect.hpp>
#include <xml/parse/type/function.hpp>
#include <xml/parse/type/place.hpp>
#include <xml/parse/type/place_map.hpp>
#include <xml/parse/type/port.hpp>
#include <xml/parse/type/template.hpp>
#include <xml/parse/type/transition.hpp>

#include <FMT/we/type/port/direction.hpp>
#include <FMT/xml/parse/util/position.hpp>
#include <fmt/core.h>
#include <we/type/net.hpp>

namespace xml
{
  namespace parse
  {
    namespace warning
    {
      port_not_connected::port_not_connected
        (type::port_type const& port, ::boost::filesystem::path const& path)
          : generic ( fmt::format ( "%1%-port {} not connected in {}"
                                  , port.direction()
                                  , port.name()
                                  , path
                                  )
                    )
          , _path (path)
      { }

      conflicting_port_types::conflicting_port_types
        ( type::transition_type const& transition
        , type::port_type const& in
        , type::port_type const& out
        , ::boost::filesystem::path const& path
        )
          : generic ( fmt::format ( "port {} of transition {} has differing "
                                    "types for input ({}) and output ({}) "
                                    "in {}"
                                  , in.name()
                                  , transition.name()
                                  , in.type()
                                  , out.type()
                                  , path
                                  )
                    )
          , _path (path)
      { }

      overwrite_function_name_trans::overwrite_function_name_trans
        (type::transition_type const& trans, type::function_type const& function)
          : generic ( fmt::format ( "name of function {} defined at {} "
                                    "overwritten with name of transition {} "
                                    "at {}"
                                   , function.name().get_value_or ("<<anonymous>>")
                                   , function.position_of_definition()
                                   , trans.name()
                                   , trans.position_of_definition()
                                   )
                    )
      { }

      duplicate_external_function::duplicate_external_function
        (type::module_type const& mod, type::module_type const& old)
          : generic ( fmt::format ( "the external function {} in module {}"
                                    " has multiple occurences in {} and {}"
                                  , mod.function()
                                  , mod.name()
                                  , old.position_of_definition()
                                  , mod.position_of_definition()
                                  )
                    )
      {}

      synthesize_anonymous_function::synthesize_anonymous_function
        (type::function_type const& function)
          : generic ( fmt::format ( "synthesize anonymous top level function"
                                    " at {}"
                                   , function.position_of_definition()
                                   )
                    )
      {}

      struct_redefined::struct_redefined ( type::structure_type const& early
                                         , type::structure_type const& late
                                         )
        : generic ( fmt::format ( "struct {} at {} redefined at {}"
                                , early.name()
                                , early.position_of_definition()
                                , late.position_of_definition()
                                )
                  )
      {}
    }
  }
}
