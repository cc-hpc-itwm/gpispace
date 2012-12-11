/*
 * =====================================================================================
 *
 *       Filename:  types.hpp
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  02/25/2010 05:06:55 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef WE_MGMT_BITS_TYPES_HPP
#define WE_MGMT_BITS_TYPES_HPP 1

#include <string>
#include <cassert>
#include <fhg/assert.hpp>
#include <we/net.hpp>
#include <we/expr/parse/parser.hpp>

namespace we { namespace mgmt {

  namespace detail {

        struct token_t
        {
          typedef std::string type;

          token_t()
          {}

          template <typename _Tp>
          explicit
          token_t(const _Tp & value_)
                : value(value_)
          {}

          token_t(const token_t &other)
                : value(other.value)
          {}

          token_t & operator=(const token_t &other)
          {
                if (&other != this)
                {
                  value = other.value;
                }
                return *this;
          }

          type value;

          friend bool operator==(const token_t &, const token_t &);
          friend std::ostream& operator<<(std::ostream &, const token_t &);
          friend std::size_t hash_value(const token_t &);
        };

        inline bool operator==(const token_t & a, const token_t & b)
        {
          return (a.value == b.value);
        }
        inline bool operator!=(const token_t & a, const token_t & b)
        {
          return !(a == b);
        }
        inline std::size_t hash_value(token_t const & t)
        {
          boost::hash<token_t::type> hasher;
          return hasher(t.value);
        }

        inline std::ostream & operator<< (std::ostream & s, const token_t & t)
        {
          return s << "t(" << t.value << ")";
        }

        struct place_t
        {
          explicit
          place_t (std::string const & name_)
                : name(name_)
          {}

          std::string name;
        };
        inline bool operator==(const place_t & a, const place_t & b)
        {
          return a.name == b.name;
        }
        inline std::size_t hash_value(place_t const & p)
        {
          boost::hash<std::string> hasher;
          return hasher(p.name);
        }
        inline std::ostream & operator<< (std::ostream & s, const place_t & p)
        {
          return s << p.name;
        }

        struct edge_t
        {
          explicit
          edge_t(std::string const & name_)
                : name(name_)
          {}

          edge_t(const edge_t &other)
          {
                name = other.name;
          }

          edge_t & operator=(const edge_t &other)
          {
                name = other.name;
                return *this;
          }

          std::string name;
        };

        inline bool operator==(const edge_t & a, const edge_t & b)
        {
          return a.name == b.name;
        }
        inline std::size_t hash_value(edge_t const & e)
        {
          boost::hash<std::string> hasher;
          return hasher(e.name);
        }
        inline std::ostream & operator<< (std::ostream & s, const edge_t & e)
        {
          return s << e.name;
        }
  }
}}

#endif
