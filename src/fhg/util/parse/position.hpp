// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#pragma once

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

      class position_vector_of_char : public position
      {
      public:
        position_vector_of_char (const std::vector<char> &);
        position_vector_of_char ( const std::vector<char>::const_iterator &begin
                                , const std::vector<char>::const_iterator &end
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
        std::vector<char>::const_iterator _pos;
        const std::vector<char>::const_iterator _begin;
        const std::vector<char>::const_iterator _end;
      };
    }
  }
}
