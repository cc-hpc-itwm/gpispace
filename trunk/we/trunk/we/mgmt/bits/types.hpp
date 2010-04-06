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
#include <we/net.hpp>
#include <we/mgmt/bits/pid_map_t.hpp>

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

    template <typename Place, typename Edge, typename Token>
	struct transition_t
	{
	  enum Category
	  {
        MOD_CALL
	  , EXPRESSION
      , NET
	  };

      typedef petri_net::net<Place, transition_t<Place, Edge, Token>, Edge, Token> net_type;
      struct mod_t
      {
        mod_t(std::string const &m_, std::string const &f_)
          : m(m_), f(f_) {}
        const std::string m;
        const std::string f;
      };

      struct expr_t
      {
        expr_t(std::string const &e)
          : v(e) {}
        const std::string v;
      };

      typedef mod_t mod_type;
      typedef expr_t expr_type;

      typedef petri_net::pid_t pid_t;
      typedef typename traits::pid_map_traits<pid_t>::type pid_map_t;
      typedef Category category_t;

      struct flags_t
      {
        bool internal : 1;
      };

      template <typename Type>
	  transition_t (const std::string & name_, Type const & typ, bool intern = false)
		: name(name_)
	  {
        data.ptr = 0;
        assign(typ);
        flags.internal = intern;
      }

      transition_t (const transition_t &other)
        : name(other.name)
        , type(other.type)
        , flags(other.flags)
        , mapping(other.mapping)
      {
        data.ptr = 0;
        if (other.data.ptr)
        {
          switch (type)
          {
            case MOD_CALL:
              assign ( *other.data.mod );
              break;
            case NET:
              assign ( *other.data.net );
              break;
            case EXPRESSION:
              assign ( *other.data.expr );
              break;
            default:
              assert(false);
          }
        }
      }

      void assign( net_type const & net )
      {
        clear();
        type = NET;
        data.net = new net_type (net);
      }

      void assign( expr_type const & expr )
      {
        clear();
        type = EXPRESSION;
        data.expr = new expr_type (expr);
      }

      void assign( mod_type const & mod )
      {
        clear();
        type = MOD_CALL;
        data.mod = new mod_type (mod);
      }

      inline
      void clear()
      {
        if (data.ptr)
        {
          switch (type)
          {
            case NET:
              delete data.net;
              break;
            case MOD_CALL:
              delete data.mod;
              break;
            case EXPRESSION:
              delete data.expr;
              break;
            default:
              assert(false);
          }
        }
        data.ptr = 0;
      }

      transition_t & operator=(const transition_t & other)
      {
        if (this != &other)
        {
          name = other.name;
          flags = other.flags;
          mapping = other.mapping;
          clear();

          type = other.type;

          if (other.data.ptr)
          {
            switch (type) // new type
            {
              case MOD_CALL:
                assign ( *other.data.mod );
                break;
              case NET:
                assign ( *other.data.net );
                break;
              case EXPRESSION:
                assign ( *other.data.expr );
                break;
              default:
                assert(false);
            }
          }
        }
        return *this;
      }

      ~transition_t ()
      {
        try
        {
          clear();
        }
        catch (...)
        {

        }
      }

	  template <typename Input, typename OutputDescription, typename Output>
	  void operator ()(Input const & input, OutputDescription const & desc, Output & output) const
	  {
		for ( typename Input::const_iterator in (input.begin())
			; in != input.end()
			; ++in)
		{
		  for ( typename OutputDescription::const_iterator out (desc.begin())
			  ; out != desc.end()
			  ; ++out)
		  {
			std::cerr << "D: putting " << in->first << " to place " << out->first << std::endl;
			output.push_back(std::make_pair(in->first, out->first));
		  }
		}
	  }

      template <typename Pid>
      void connect(const Pid outer, const Pid inner)
      {
        mapping.insert (pid_map_t::value_type(outer, inner));
      }

      union
      {
        net_type *net;
        expr_type *expr;
        mod_type *mod;
        void *ptr;
      } data;

	  std::string name;
	  category_t type;
      flags_t flags;
      pid_map_t mapping;
	};
    template <typename P, typename E, typename T>
	inline bool operator==(const transition_t<P,E,T> & a, const transition_t<P,E,T> & b)
	{
	  return a.name == b.name;
	}
    template <typename P, typename E, typename T>
	inline std::size_t hash_value(transition_t<P,E,T> const & t)
	{
	  boost::hash<std::string> hasher;
	  return hasher(t.name);
	}
    template <typename P, typename E, typename T>
	inline std::ostream & operator<< (std::ostream & s, const transition_t<P,E,T> & t)
	{
	  return s << t.name;
	}
  }
}}

#endif
