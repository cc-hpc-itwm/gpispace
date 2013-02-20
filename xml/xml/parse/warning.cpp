// bernd.loerwald@itwm.fraunhofer.de

#include <xml/parse/warning.hpp>

#include <xml/parse/type/connect.hpp>
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
        (const id::ref::port& port, const boost::filesystem::path& path)
          : generic ( boost::format ("%1%-port %2% not connected in %3%")
                    % we::type::enum_to_string (port.get().direction())
                    % port.get().name()
                    % path
                    )
          , _port (port)
          , _path (path)
      { }

      conflicting_port_types::conflicting_port_types
        ( const id::ref::transition& transition
        , const id::ref::port& in
        , const id::ref::port& out
        , const boost::filesystem::path& path
        )
          : generic ( boost::format ( "port %1% of transition %2% has differing "
                                      "types for input (%3%) and output (%4%) "
                                      "in %5%"
                                    )
                    % in.get().name()
                    % transition.get().name()
                    % in.get().type()
                    % out.get().type()
                    % path
                    )
          , _transition (transition)
          , _in (in)
          , _out (out)
          , _path (path)
      { }

      overwrite_function_name_trans::overwrite_function_name_trans
        (const id::ref::transition& trans, const id::ref::function& function)
          : generic ( boost::format ( "name of function %1% defined in %2% "
                                      "overwritten with name of transition %3% "
                                      "in %4%"
                                    )
                    % function.get().name().get_value_or ("<<anonymous>>")
                    % function.get().path
                    % trans.get().name()
                    % trans.get().path
                    )
          , _transition (trans)
          , _function (function)
      { }

      overwrite_function_internal_trans::overwrite_function_internal_trans
        (const id::ref::transition& trans, const id::ref::function& function)
          : generic ( boost::format ( "transition %1% in %2% overwrites the "
                                      "internal tag of the contained function"
                                    )
                    % trans.get().name()
                    % trans.get().path
                    )
          , _transition (trans)
          , _function (function)
      { }
    }
  }
}
