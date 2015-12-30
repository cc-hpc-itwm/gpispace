// mirko.rahn@itwm.fraunhofer.de

#include <fhg/util/parse/position.hpp>

#include <fhg/util/parse/error.hpp>
#include <fhg/util/parse/require.hpp>

#include <sstream>

namespace fhg
{
  namespace util
  {
    namespace parse
    {
      position_string::position_string (const std::string& input)
        : _k (0)
        , _pos (input.begin())
        , _begin (input.begin())
        , _end (input.end())
      {}
      position_string::position_string ( const std::string::const_iterator& begin
                                       , const std::string::const_iterator& end
                                       )
        : _k (0)
        , _pos (begin)
        , _begin (begin)
        , _end (end)
      {}

      std::string position_string::error_message (const std::string& message) const
      {
        std::ostringstream oss;

        oss << "PARSE ERROR [" << eaten() << "]: " << message << std::endl;
        oss << std::string (_begin, _pos) << ' '
            << std::string (_pos, _end) << std::endl;
        oss << std::string (eaten(), ' ') << "^" << std::endl;

        return oss.str();
      }
    }
  }
}

namespace fhg
{
  namespace util
  {
    namespace parse
    {
      position_vector_of_char::position_vector_of_char (const std::vector<char>& input)
        : _k (0)
        , _pos (input.begin())
        , _begin (input.begin())
        , _end (input.end())
      {}
      position_vector_of_char::position_vector_of_char ( const std::vector<char>::const_iterator &begin
                                                       , const std::vector<char>::const_iterator &end
                                                       )
        : _k (0)
        , _pos (begin)
        , _begin (begin)
        , _end (end)
      {}

      std::string position_vector_of_char::error_message (const std::string& message) const
      {
        std::ostringstream oss;

        oss << "PARSE ERROR [" << eaten() << "]: " << message << std::endl;
        oss << std::string (_begin, _pos) << ' '
            << std::string (_pos, _end) << std::endl;
        oss << std::string (eaten(), ' ') << "^" << std::endl;

        return oss.str();
      }
    }
  }
}

namespace fhg
{
  namespace util
  {
    namespace parse
    {
      position_istream::position_istream (std::istream& input)
        : _k (0)
        , _pos (input)
      {
      }

      std::string position_istream::error_message (const std::string& message) const
      {
        std::ostringstream oss;

        oss << "PARSE ERROR [" << eaten() << "]: " << message << std::endl;

        return oss.str();
      }
    }
  }
}
