// bernd.loerwald@itwm.fraunhofer.de

#include <xml/parse/error.hpp>

#include <xml/parse/type/connect.hpp>
#include <xml/parse/type/transition.hpp>

#include <boost/format.hpp>

namespace xml
{
  namespace parse
  {
    namespace error
    {
      duplicate_connect::duplicate_connect
        ( const std::string& type
        , const id::ref::connect& connection
        , const id::ref::connect& old_connection
        , const id::ref::transition& transition
        , const boost::filesystem::path& path
        )
          : generic ( boost::format ( "duplicate connect-%1% %2% <-> %3% "
                                      "for transition %4% in %5%"
                                    )
                    % type
                    % connection.get().place()
                    % connection.get().port()
                    % transition.get().name()
                    % path
                    )
        , _type (type)
        , _connection (connection)
        , _old_connection (old_connection)
        , _transition (transition)
        , _path (path)
      { }
    }
  }
}
