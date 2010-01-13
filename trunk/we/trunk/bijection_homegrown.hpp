#ifndef _BIJECTION_HPP
#define _BIJECTION_HPP

// keep a bijection between a set of objects and some ids
// basically implementing a simple hash map, keeping the reverse too
// mirko.rahn@itwm.fraunhofer.de

#include <tr1/functional> // hash
#include <map>            // map, multimap

#include <stdint.h>       // unit64_t
#include <limits>         // numeric_limits
#include <stdexcept>      // std::runtime_error

#include <cassert>        // assert

namespace bijection
{
  // Martin Kühn: If you aquire a new handle each cycle, then, with 3e9
  // cycles per second, you can run for 2^64/3e9/60/60/24/365 ~ 195 years.
  // It follows that an uint64_t is enough for now.

  typedef uint64_t id_t;

  typedef std::size_t hash_t;

  static const id_t invalid (std::numeric_limits<id_t>::max());

  struct handle_t
  {
  private:
    id_t value;
  public:
    handle_t () throw () : value(std::numeric_limits<id_t>::min()) {}
    const id_t operator * (void) throw () { return value++; }
  };

  class already_there : public std::runtime_error
  {
  public:
    already_there (const std::string & msg) : std::runtime_error (msg) {}
    ~already_there (void) throw () {}
  };

  class no_such : public std::runtime_error
  {
  public:
    no_such (const std::string & msg) : std::runtime_error (msg) {}
    ~no_such (void) throw () {}
  };

  template<class T, class Hash = std::tr1::hash<T> >
  class bijection
  {
  private:
    handle_t h;

    typedef typename std::map<id_t,T> id_to_obj_t;
    typedef std::multimap<hash_t,id_t> hash_to_id_t;
    typedef hash_to_id_t::const_iterator hash_to_id_const_it;
    typedef std::pair< hash_to_id_const_it
                     , hash_to_id_const_it> hash_to_id_range_const_it;
    typedef hash_to_id_t::iterator hash_to_id_it;
    typedef std::pair< hash_to_id_it
                     , hash_to_id_it> hash_to_id_range_it;

    id_to_obj_t id_to_obj;
    hash_to_id_t hash_to_id;

    const std::string description;

    const id_t get_id_from_hash (const T & x, const hash_t & hash) const
      throw (no_such)
    {
      const hash_to_id_range_const_it range_const_it
        (hash_to_id.equal_range (hash));

      for ( hash_to_id_const_it id_it (range_const_it.first)
          ; id_it != range_const_it.second
          ; ++id_it
          )
        {
          const id_t id (id_it->second);

          const typename id_to_obj_t::const_iterator obj_it
            (id_to_obj.find (id));

          assert (obj_it != id_to_obj.end());

          if (obj_it->second == x)
            return id;
        }

      throw no_such (description);
    }

    const id_t insert_with_hash (const T & x, const hash_t & hash) throw ()
    {
      const id_t id (*h);

      assert (id_to_obj.find(id) == id_to_obj.end());

      id_to_obj[id] = x;
      hash_to_id.insert (hash_to_id_t::value_type (hash, id));

      return id;
    }

    void erase_with_hash (const id_t & id, const hash_t & hash) throw ()
    {
      id_to_obj.erase (id);

      const hash_to_id_range_it range_it (hash_to_id.equal_range (hash));

      for ( hash_to_id_it id_it (range_it.first)
          ; id_it != range_it.second
          ; ++id_it
          )
        if (id_it->second == id)
          {
            hash_to_id.erase (id_it);

            return;
          }

      assert (false);
    }

  public:
    typedef typename id_to_obj_t::const_iterator const_it;

    const const_it begin (void) const throw () { return id_to_obj.begin(); }
    const const_it end (void) const throw () { return id_to_obj.end(); }

    bijection (const std::string & descr = "NO_DESCRIPTION_GIVEN") throw ()
      : description(descr)
    {}

    const id_t unsafe_insert (const T & x) throw ()
    {
      return insert_with_hash (x, Hash()(x));
    }

    const id_t insert (const T & x) throw (already_there)
    {
      const hash_t hash (Hash()(x));

      try
        {
          get_id_from_hash (x, hash);
        }
      catch (no_such)
        {
          return insert_with_hash (x, hash);
        }

      throw already_there (description);
    }

    const T & get_elem (const id_t & id) const throw (no_such)
    {
      typename id_to_obj_t::const_iterator obj_it (id_to_obj.find (id));

      if (obj_it == id_to_obj.end())
        throw no_such (description);

      return obj_it->second;
    }

    const id_t get_id (const T & x) const throw (no_such)
    {
      return get_id_from_hash (x, Hash()(x));
    }

    void erase (const T & x) throw (no_such)
    {
      const hash_t hash (Hash()(x));

      erase_with_hash (get_id_from_hash (x, hash), hash);
    }

    template<class U, class H>
    friend std::ostream & operator << (std::ostream &, const bijection<U, H> &);
  };

  template<class U, class H>
  std::ostream & operator << (std::ostream & s, const bijection<U, H> & b)
  {
    s << "**** bijection (" << b.description << ")" << std::endl;

    s << "** id -> obj" << std::endl;

    for ( typename bijection<U, H>::id_to_obj_t::const_iterator it (b.id_to_obj.begin())
        ; it != b.id_to_obj.end()
        ; ++it
        )
      s << it->first << " => " << it->second << std::endl;

    s << "** hash -> id" << std::endl;

    for ( typename bijection<U, H>::hash_to_id_t::const_iterator it (b.hash_to_id.begin())
        ; it != b.hash_to_id.end()
        ; ++it
        )
      s << it->first << " => " << it->second << std::endl;

    return s;
  }
}

#endif // _BIJECTION_HPP
