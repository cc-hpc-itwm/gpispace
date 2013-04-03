// mirko.rahn@itwm.fraunhofer.de

#include <fhg/util/parse/position.hpp>
#include <fhg/util/parse/error.hpp>

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
      void position::require (const std::string& what)
      {
        std::string::const_iterator what_pos (what.begin());
        const std::string::const_iterator what_end (what.end());

        while (what_pos != what_end)
        {
          if (end() || operator*() != *what_pos)
          {
            throw error::expected (std::string (what_pos, what_end), *this);
          }
          else
          {
            operator++(); ++what_pos;
          }
        }
      }
    }
  }
}
