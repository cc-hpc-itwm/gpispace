// bernd.loerwald@itwm.fraunhofer.de

#include <xml/parse/error.hpp>

#include <boost/format.hpp>

namespace xml
{
  namespace parse
  {
    namespace error
    {
      duplicate_connect::duplicate_connect
        ( const std::string & type
        , const std::string & name
        , const std::string & trans
        , const boost::filesystem::path & path
        )
          : generic ( boost::format ( "duplicate connect-%1% %2% "
                                      "for transition %3% in %4%"
                                    )
                    % type
                    % name
                    % trans
                    % path
                    )
      { }
    }
  }
}
