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
      void position::require (const char& c)
      {
        require (std::string (1, c));
      }

      std::string position::identifier()
      {
        skip_spaces();

        std::string id;

        if (end() || !(isalpha (*_pos) || *_pos == '_'))
        {
          throw error::expected ("identifier [a-zA-Z_][a-zA-Z_0-9]*", *this);
        }

        id.push_back (*_pos); operator++();

        while (!end() && (isalpha (*_pos) || *_pos == '_' || isdigit (*_pos)))
        {
          id.push_back (*_pos); operator++();
        }

        return id;
      }

      namespace
      {
        char unquote (char)
        {
          throw std::runtime_error ("Quoted characters are not yet supported");
        }
      }

      char position::character()
      {
        if (end())
        {
          throw error::expected ("character", *this);
        }

        bool quoted (false);

        if (*_pos == '\\')
        {
          operator++();

          quoted = true;
        }

        if (end())
        {
          throw error::expected ("character", *this);
        }

        const char c (quoted ? unquote (*_pos) : *_pos);

        operator++();

        return c;
      }

      std::string position::until (const char c, const char escape)
      {
        std::string s;

        while (!end() && *_pos != c)
        {
          if (*_pos == escape)
          {
            operator++();

            if (!end() && (*_pos == c || *_pos == escape))
            {
              s.push_back (*_pos);
              operator++();
            }
            else
            {
              throw error::expected ( std::string (1, c)
                                    + " or "
                                    + std::string (1, escape)
                                    , *this
                                    );
            }
          }
          else
          {
            s.push_back (character());
          }
        }

        if (end())
        {
          throw error::expected (std::string (1, c), *this);
        }

        operator++();

        return s;
      }

      void position::list ( const char open, const char sep, const char close
                          , const boost::function<void (position&)>& f
                          , const bool skip_space_before_element
                          , const bool skip_space_after_element
                          )
      {
        skip_spaces();

        require (open);

        if (skip_space_before_element)
        {
          skip_spaces();
        }

        bool closed (false);
        bool expect_sep (false);

        do
        {
          if (end())
          {
            throw error::expected
              ( std::string (1, close)
              + " or "
              + (expect_sep ? std::string (1, sep) : "<list_element>")
              , *this
              );
          }

          if (*_pos == close)
          {
            operator++();

            closed = true;
          }
          else if (expect_sep)
          {
            require (sep);

            if (skip_space_before_element)
            {
              skip_spaces();
            }

            expect_sep = false;
          }
          else if (*_pos == sep)
          {
            throw error::expected ("<list_element>", *this);
          }
          else
          {
            f (*this);

            if (skip_space_after_element)
            {
              skip_spaces();
            }

            expect_sep = true;
          }
        }
        while (!closed);
      }
    }
  }
}
