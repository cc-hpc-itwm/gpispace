// mirko.rahn@itwm.fraunhofer.de

#include <fhg/util/parse/position.hpp>

#include <fhg/util/parse/error.hpp>
#include <fhg/util/parse/require.hpp>

namespace fhg
{
  namespace util
  {
    namespace parse
    {
      position::position (const std::string& input)
        : _k (0)
        , _pos (input.begin())
        , _begin (input.begin())
        , _end (input.end())
      {}
      position::position ( const std::string::const_iterator& begin
                         , const std::string::const_iterator& end
                         )
        : _k (0)
        , _pos (begin)
        , _begin (begin)
        , _end (end)
      {}

      std::string position::consumed() const
      {
        return std::string (_begin, _pos);
      }
      std::string position::rest() const
      {
        return std::string (_pos, _end);
      }
      const std::size_t& position::operator() () const
      {
        return _k;
      }
      void position::skip_spaces()
      {
        require::skip_spaces (*this);
      }
      void position::require (const std::string& what)
      {
        require::require (*this, what);
      }
      void position::require (const char& c)
      {
        require::require (*this, c);
      }
      std::string position::until (const char c, const char escape)
      {
        return require::plain_string (*this, c, escape);
      }
      void position::list ( const char open, const char sep, const char close
                          , const boost::function<void (position&)>& fun
                          , const bool skip_before
                          , const bool skip_after
                          )
      {
        require::list (*this, open, sep, close, fun, skip_before, skip_after);
      }
    }
  }
}
