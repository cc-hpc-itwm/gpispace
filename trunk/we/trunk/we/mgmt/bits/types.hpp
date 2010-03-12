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

namespace we { namespace mgmt {

  namespace detail {

	struct token_t
	{
	  typedef std::string type;

	  token_t()
		: id(counter()++)
	  {}

	  template <typename _Tp>
	  token_t(const _Tp & value_)
		: id(counter()++)
		, value(value_)
	  {}

	  token_t(const token_t &other)
	  {
		value = other.value;
		id = other.id;
	  }

	  token_t & operator=(const token_t &other)
	  {
		value = other.value;
		id = other.id;
		return *this;
	  }

	  unsigned long long id;
	  type value;
	private:
	  static unsigned long long& counter()
	  {
		static unsigned long long counter = 0;
		return counter;
	  }
	};

	inline bool operator==(const token_t & a, const token_t & b)
	{
	  return a.id == b.id;
	}
	inline bool operator!=(const token_t & a, const token_t & b)
	{
	  return !(a == b);
	}
	inline std::size_t hash_value(token_t const & t)
	{
	  boost::hash<unsigned long long> hasher;
	  return hasher(t.id);
	}
	inline std::ostream & operator<< (std::ostream & s, const token_t & t)
	{
	  return s << t.value;
	}


	struct place_t
	{
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

	struct transition_t
	{
	  enum transition_cat
	  {
		INTERNAL_SIMPLE
	  , INTERNAL_COMPLX
	  , EXTERNAL
	  };

	  transition_t (const std::string & name_, transition_cat category_)
		: name(name_)
		, type(category_)
	  {}

	  std::string name;
	  transition_cat type;
	};
	inline bool operator==(const transition_t & a, const transition_t & b)
	{
	  return a.name == b.name;
	}
	inline std::size_t hash_value(transition_t const & t)
	{
	  boost::hash<std::string> hasher;
	  return hasher(t.name);
	}
	inline std::ostream & operator<< (std::ostream & s, const transition_t & t)
	{
	  return s << t.name;
	}

	struct edge_t
	{
	  explicit
	  edge_t(std::string const & name_)
		: id(counter()++)
		, name(name_)
	  {}

	  edge_t(const edge_t &other)
	  {
		name = other.name;
		id = other.id;
	  }

	  edge_t & operator=(const edge_t &other)
	  {
		name = other.name;
		id = other.id;
		return *this;
	  }

	  unsigned long long id;
	  std::string name;
	private:
	  static unsigned long long& counter()
	  {
		static unsigned long long counter = 0;
		return counter;
	  }
	};

	inline bool operator==(const edge_t & a, const edge_t & b)
	{
	  return a.id == b.id;
	}
	inline std::size_t hash_value(edge_t const & e)
	{
	  boost::hash<unsigned long long> hasher;
	  return hasher(e.id);
	}
	inline std::ostream & operator<< (std::ostream & s, const edge_t & e)
	{
	  return s << e.name;
	}
  }
}}

#endif
