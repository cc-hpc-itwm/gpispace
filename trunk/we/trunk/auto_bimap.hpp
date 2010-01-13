#ifndef _AUTOBIMAP_HPP
#define _AUTOBIMAP_HPP

// bimap, that keeps a bijection between objects and some index
// mirko.rahn@itwm.fraunhofer.de

#include <ostream>

#include <boost/bimap.hpp>
#include <boost/bimap/unordered_set_of.hpp>

#include <handle.hpp>

namespace auto_bimap
{
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

  template<typename T, typename I = handle::T>
  class auto_bimap
  {
  private:
    // unordered, unique, viewable
    typedef boost::bimaps::unordered_set_of<T> elem_collection_t;

    // unordered, unique, viewable
    typedef boost::bimaps::unordered_set_of<I> id_collection_t;

    typedef boost::bimap<elem_collection_t, id_collection_t> bimap_t;
    typedef typename bimap_t::value_type val_t;

    bimap_t bimap;
    I h;
    const std::string description;

  public:
    auto_bimap (const std::string & _description = "NO DESCRIPTION GIVEN")
      : h(0)
      , description (_description)
    {}

    typedef typename bimap_t::iterator iterator;

    iterator begin (void) { return bimap.begin(); }
    iterator end (void) { return bimap.end(); }

    typedef typename bimap_t::const_iterator const_iterator;

    const const_iterator begin (void) const { return bimap.begin(); }
    const const_iterator end (void) const { return bimap.end(); }

    const I & get_id (const T & x) const throw (exception::no_such)
    {
      typename bimap_t::left_map::const_iterator it (bimap.left.find (x));

      if (it == bimap.left.end())
        throw exception::no_such (description);

      return it->second;
    }

    const T & get_elem (const I & i) const throw (exception::no_such)
    {
      typename bimap_t::right_map::const_iterator it (bimap.right.find (i));

      if (it == bimap.right.end())
        throw exception::no_such ("index for " + description);

      return it->second;
    }

    const I add (const T & x) throw (exception::already_there)
    {
      if (bimap.left.find (x) != bimap.left.end())
        throw exception::already_there (description);

      I i (h++);

      bimap.insert (val_t (x, i));

      return i;
    }

    const void erase (const I & i)
    {
      bimap.right.erase (i);
    }

    const I modify (const I & i, const T & x)
      throw (exception::no_such, exception::already_there)
    {
      typename bimap_t::right_map::iterator it (bimap.right.find (i));

      if (it == bimap.right.end())
        throw exception::no_such ("index for " + description);

      if (bimap.right.modify_data (it, boost::bimaps::_data = x) == false)
        throw exception::already_there (description + " after modify");

      return i;
    }

    const I replace (const I & i, const T & x)
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

  template<typename T, typename I = handle::T>
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
    const I & operator * (void) const { return pos->right; }
    const std::size_t count (void) const { return count_; }
  };
} // namespace auto_bimap

#endif // _AUTOBIMAP_HPP
