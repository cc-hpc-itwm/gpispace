// bernd.loerwald@itwm.fraunhofer.de

#include <xml/parse/error.hpp>

#include <xml/parse/type/connect.hpp>
#include <xml/parse/type/place_map.hpp>
#include <xml/parse/type/template.hpp>
#include <xml/parse/type/transition.hpp>

#include <we/net.hpp>

#include <boost/format.hpp>

namespace xml
{
  namespace parse
  {
    namespace error
    {
      duplicate_connect::duplicate_connect
        ( const id::ref::connect& connection
        , const id::ref::connect& old_connection
        , const id::ref::transition& transition
        , const boost::filesystem::path& path
        )
          : generic ( boost::format ( "duplicate connect-%1% %2% <-> %3% "
                                      "for transition %4% in %5%"
                                      " (existing connection is connect-%6%)"
                                    )
                    % petri_net::edge::enum_to_string
                      (connection.get().direction())
                    % connection.get().place()
                    % connection.get().port()
                    % transition.get().name()
                    % path
                    % petri_net::edge::enum_to_string
                      (old_connection.get().direction())
                    )
          , _connection (connection)
          , _old_connection (old_connection)
          , _transition (transition)
          , _path (path)
      { }

      duplicate_place_map::duplicate_place_map
        ( const id::ref::place_map& place_map
        , const id::ref::place_map& old_place_map
        , const id::ref::transition& transition
        , const boost::filesystem::path& path
        )
          : generic ( boost::format ( "duplicate place-map %1% <-> %2% for "
                                      "transition %3% in %4%"
                                    )
                    % place_map.get().place_virtual()
                    % place_map.get().place_real()
                    % transition.get().name()
                    % path
                    )
          , _place_map (place_map)
          , _old_place_map (old_place_map)
          , _transition (transition)
          , _path (path)
      { }

      duplicate_transition::duplicate_transition
        ( const id::ref::transition& transition
        , const id::ref::transition& old_transition
        )
          : generic ( boost::format ( "duplicate transition %1% in %2%. "
                                      "first definition was in %3%"
                                    )
                    % transition.get().name()
                    % transition.get().path
                    % old_transition.get().path
                    )
          , _transition (transition)
          , _old_transition (old_transition)
      { }

      duplicate_function::duplicate_function
        ( const id::ref::function& function
        , const id::ref::function& old_function
        )
          : generic ( boost::format ( "duplicate function %1% in %2%. "
                                      "first definition was in %3%"
                                    )
                    % function.get().name()
                    % function.get().path
                    % old_function.get().path
                    )
          , _function (function)
          , _old_function (old_function)
      { }

      duplicate_template::duplicate_template
        ( const id::ref::tmpl& tmpl
        , const id::ref::tmpl& old_template
        )
          : generic ( boost::format ( "duplicate template %1% in %2%. "
                                      "first definition was in %3%"
                                    )
                    % tmpl.get().name()
                    % tmpl.get().path()
                    % old_template.get().path()
                    )
          , _template (tmpl)
          , _old_template (old_template)
      { }
    }
  }
}
