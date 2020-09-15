#pragma once

#include <cstddef>
#include <iterator>
#include <string>

namespace fhg
{
  namespace iml
  {
    namespace util
    {
      namespace parse
      {
        class position
        {
        public:
          virtual ~position() = default;

          virtual char operator*() const = 0;
          virtual void operator++() = 0;
          virtual void advance (std::size_t) = 0;
          virtual bool end() const = 0;
          virtual std::size_t eaten() const = 0;
          virtual std::string eat (std::size_t) = 0;

          virtual std::string error_message (const std::string&) const = 0;
        };

        class position_string : public position
        {
        public:
          position_string (const std::string&);
          position_string ( const std::string::const_iterator& begin
                          , const std::string::const_iterator& end
                          );

          virtual char operator*() const override
          {
            return *_pos;
          }
          virtual void operator++() override
          {
            ++_k;
            ++_pos;
          }
          virtual void advance (std::size_t d) override
          {
            _k += d;
            std::advance (_pos, d);
          }
          virtual bool end() const override
          {
            return _pos == _end;
          }
          virtual std::size_t eaten() const override
          {
            return _k;
          }
          virtual std::string eat (std::size_t n) override
          {
            std::string const ret (_pos, _pos + n);
            advance (n);
            return ret;
          }

          virtual std::string error_message (const std::string&) const override;

        private:
          std::size_t _k;
          std::string::const_iterator _pos;
          const std::string::const_iterator _begin;
          const std::string::const_iterator _end;
        };
      }
    }
  }
}
