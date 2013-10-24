// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_UTIL_PARSE_POSITION_HPP
#define _FHG_UTIL_PARSE_POSITION_HPP

#include <iterator>
#include <string>
#include <vector>

namespace fhg
{
  namespace util
  {
    namespace parse
    {
      class position
      {
      public:
        virtual ~position() {}

        virtual char operator*() const = 0;
        virtual void operator++() = 0;
        virtual void advance (std::size_t) = 0;
        virtual bool end() const = 0;
        virtual std::size_t eaten() const = 0;

        virtual std::string error_message (const std::string&) const = 0;
      };

      class position_string : public position
      {
      public:
        position_string (const std::string&);
        position_string ( const std::string::const_iterator& begin
                        , const std::string::const_iterator& end
                        );

        virtual char operator*() const
        {
          return *_pos;
        }
        virtual void operator++()
        {
          ++_k;
          ++_pos;
        }
        virtual void advance (std::size_t d)
        {
          _k += d;
          std::advance (_pos, d);
        }
        virtual bool end() const
        {
          return _pos == _end;
        }
        virtual std::size_t eaten() const
        {
          return _k;
        }

        virtual std::string error_message (const std::string&) const;

      private:
        std::size_t _k;
        std::string::const_iterator _pos;
        const std::string::const_iterator _begin;
        const std::string::const_iterator _end;
      };

      class position_vector_of_char : public position
      {
      public:
        position_vector_of_char (const std::vector<char> &);
        position_vector_of_char ( const std::vector<char>::const_iterator &begin
                                , const std::vector<char>::const_iterator &end
                                );

        virtual char operator*() const
        {
          return *_pos;
        }
        virtual void operator++()
        {
          ++_k;
          ++_pos;
        }
        virtual void advance (std::size_t d)
        {
          _k += d;
          std::advance (_pos, d);
        }
        virtual bool end() const
        {
          return _pos == _end;
        }
        virtual std::size_t eaten() const
        {
          return _k;
        }

        virtual std::string error_message (const std::string&) const;

      private:
        std::size_t _k;
        std::vector<char>::const_iterator _pos;
        const std::vector<char>::const_iterator _begin;
        const std::vector<char>::const_iterator _end;
      };

      class position_istream : public position
      {
      public:
        position_istream (std::istream&);

        virtual char operator*() const
        {
          return *_pos;
        }
        virtual void operator++()
        {
          ++_k;
          ++_pos;
        }
        virtual void advance (std::size_t d)
        {
          _k += d;
          std::advance (_pos, d);
        }
        virtual bool end() const
        {
          return _pos == std::istream_iterator<char>();
        }
        virtual std::size_t eaten() const
        {
          return _k;
        }

        virtual std::string error_message (const std::string&) const;

      private:
        std::size_t _k;
        std::istream_iterator<char> _pos;
      };
    }
  }
}

#endif
