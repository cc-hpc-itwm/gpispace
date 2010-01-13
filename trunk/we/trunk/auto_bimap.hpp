#ifndef _AUTOBIMAP_HPP
#define _AUTOBIMAP_HPP

// bimap, that keeps a bijection between objects and some index
// mirko.rahn@itwm.fraunhofer.de

#include <ostream>
#include <iomanip>

#include <map>
#include <set>
#include <vector>

#include <algorithm>

#include <boost/bimap.hpp>
#include <boost/bimap/unordered_set_of.hpp>
#include <boost/bimap/unordered_multiset_of.hpp>

#include <boost/typeof/typeof.hpp>
#include <boost/bimap/support/lambda.hpp>

#include <boost/function.hpp>

namespace auto_bimap
{
  // Martin Kühn: If you aquire a new handle each cycle, then, with 3e9
  // cycles per second, you can run for 2^64/3e9/60/60/24/365 ~ 195 years.
  // It follows that an uint64_t is enough for now.
  namespace handle
  {
    typedef uint64_t T;
    static const T invalid (std::numeric_limits<T>::max());

    struct handle_t
    {
    private:
      T v;
    public:
      handle_t () : v(std::numeric_limits<uint64_t>::min()) {}
      const T & operator * (void) const { return v; }
      const void operator ++ (void) { ++v; }
    };
  };

  namespace exception
  {
    class no_such : public std::runtime_error
    {
    public:
      no_such (const std::string & msg) : std::runtime_error(msg) {}
      ~no_such() throw() {}
    };

    class already_there : public std::runtime_error
    {
    public:
      already_there (const std::string & msg) : std::runtime_error(msg) {}
      ~already_there() throw() {}
    };
  } // namespace exception

  template<typename T>
  class auto_bimap
  {
  private:
    // unordered, unique, viewable
    typedef boost::bimaps::unordered_set_of<T> elem_collection_t;

    // unordered, unique, viewable
    typedef boost::bimaps::unordered_set_of<handle::T> id_collection_t;

    typedef boost::bimap<elem_collection_t, id_collection_t> bimap_t;
    typedef typename bimap_t::value_type val_t;

    bimap_t bimap;
    handle::handle_t h;

    const std::string description;

  public:
    auto_bimap (const std::string & descr = "NO DESCRIPTION GIVEN") 
      : description (descr) 
    {}

    typedef typename bimap_t::iterator iterator;

    iterator begin (void) { return bimap.begin(); }
    iterator end (void) { return bimap.end(); }

    typedef typename bimap_t::const_iterator const_iterator;

    const const_iterator begin (void) const { return bimap.begin(); }
    const const_iterator end (void) const { return bimap.end(); }

    const handle::T & get_id (const T & x) const throw (exception::no_such)
    {
      typename bimap_t::left_map::const_iterator it (bimap.left.find (x));

      if (it == bimap.left.end())
        throw exception::no_such (description);

      return it->second;
    }

    const T & get_elem (const handle::T & i) const throw (exception::no_such)
    {
      typename bimap_t::right_map::const_iterator it (bimap.right.find (i));

      if (it == bimap.right.end())
        throw exception::no_such ("index for " + description);

      return it->second;
    }

    const handle::T add (const T & x) throw (exception::already_there)
    {
      if (bimap.left.find (x) != bimap.left.end())
        throw exception::already_there (description);

      handle::T i (*h); ++h;

      bimap.insert (val_t (x, i));

      return i;
    }

    const void erase (const handle::T & i)
    {
      bimap.right.erase (i);
    }

    const handle::T modify (const handle::T & i, const T & x)
      throw (exception::no_such, exception::already_there)
    {
      typename bimap_t::right_map::iterator it (bimap.right.find (i));

      if (it == bimap.right.end())
        throw exception::no_such ("index for " + description);

      if (bimap.right.modify_data (it, boost::bimaps::_data = x) == false)
        throw exception::already_there (description + " after modify");

      return i;
    }

    const handle::T replace (const handle::T & i, const T & x)
      throw (exception::no_such, exception::already_there)
    {
      typename bimap_t::right_map::iterator it (bimap.right.find (i));

      if (it == bimap.right.end())
        throw exception::no_such ("index for " + description);

      if (bimap.right.replace_data (it, x) == false)
        throw exception::already_there (description + " after replace");

      return i;
    }

    template<typename U>
    friend std::ostream & operator << (std::ostream &, const auto_bimap<U> &);
  };

template<typename T>
std::ostream & operator << ( std::ostream & s
                           , const auto_bimap<T> & bm
                           )
{
  typedef typename auto_bimap<T>::const_iterator bm_it;

  s << "bimap (" << bm.description << "):" << std::endl;

  for (bm_it it (bm.begin()), it_end (bm.end()); it != it_end; ++it)
    s << " -- " << it->left << " <=> " << it->right << std::endl;

  return s;
};

  template<typename T>
  struct bi_const_it
  {
  private:
    typedef typename auto_bimap<T>::const_iterator it;
    it pos;
    const it end;
    const std::size_t count_;
  public:
    bi_const_it (const auto_bimap<T> & bm)
      : pos (bm.begin())
      , end (bm.end())
      , count_(std::distance(pos, end))
    {}

    const bool has_more (void) const { return (pos != end) ? true : false; }
    void operator ++ (void) { ++pos; }
    const handle::T & operator * (void) const { return pos->right; }
    const std::size_t count (void) const { return count_; }
  };
} // namespace auto_bimap

#endif // _AUTOBIMAP_HPP
