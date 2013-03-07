// mirko.rahn@itwm.fraunhofer.de

#include <fhg/util/parse/position.hpp>

namespace fhg
{
  namespace util
  {
    namespace parse
    {
      position::position ( std::size_t& k
                         , std::string::const_iterator& begin
                         , const std::string::const_iterator& end
                         )
        : _k (k)
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
      const char& position::operator*() const
      {
        return *_pos;
      }
      void position::operator++()
      {
        ++_k;
        ++_pos;
      }
      bool position::end() const
      {
        return _pos == _end;
      }
      const std::size_t& position::operator() () const
      {
        return _k;
      }
      void position::skip_spaces()
      {
        while (_pos != _end && isspace (*_pos))
        {
          ++_pos;
        }
      }
    }
  }
}
