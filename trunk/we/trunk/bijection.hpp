// bijection between objects and some index, mirko.rahn@itwm.fraunhofer.de

#ifndef _BIJECTION_HPP
#define _BIJECTION_HPP

#include <util.hpp>

#include <boost/bimap.hpp>
#include <boost/bimap/support/lambda.hpp>
#include <boost/bimap/unordered_set_of.hpp>

namespace bijection
{
  namespace exception
  {
    class no_such : public std::runtime_error
    {
    public:
      explicit no_such (const std::string & msg) : std::runtime_error(msg) {}
      ~no_such() throw() {}
    };

    class already_there : public std::runtime_error
    {
    public:
      explicit already_there (const std::string & msg) 
        : std::runtime_error(msg)
      {}
      ~already_there() throw() {}
    };
  } // namespace exception

  template<typename T, typename I>
  class bijection
  {
  private:
    // unordered, unique, viewable
    typedef boost::bimaps::unordered_set_of<T> elem_collection_t;

    // unordered, unique, viewable
    typedef boost::bimaps::unordered_set_of<I> id_collection_t;

    typedef boost::bimap<elem_collection_t, id_collection_t> bimap_t;

    bimap_t bimap;
    I h;
    const std::string description;

  public:
    explicit bijection (const std::string & _descr = "NO DESCRIPTION GIVEN")
      : h(static_cast<I>(0))
      , description (_descr)
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

      bimap.insert (typename bimap_t::value_type (x, i));

      return i;
    }

    void erase (const I & i)
    {
      bimap.right.erase (i);
    }

    I modify (const I & i, const T & x)
      throw (exception::no_such, exception::already_there)
    {
      typename bimap_t::right_map::iterator it (bimap.right.find (i));

      if (it == bimap.right.end())
        throw exception::no_such ("index for " + description);

      if (bimap.right.modify_data (it, boost::bimaps::_data = x) == false)
        throw exception::already_there (description + " after modify");

      return i;
    }

    I replace (const I & i, const T & x)
      throw (exception::no_such, exception::already_there)
    {
      typename bimap_t::right_map::iterator it (bimap.right.find (i));

      if (it == bimap.right.end())
        throw exception::no_such ("index for " + description);

      if (bimap.right.replace_data (it, x) == false)
        throw exception::already_there (description + " after replace");

      return i;
    }
  };

  template<typename T,typename I>
  struct const_it : public util::it<typename bijection<T, I>::const_iterator>
  {
  private:
    typedef util::it<typename bijection<T, I>::const_iterator> super;
  public:
    const_it (const bijection<T,I> & b) : super (b.begin(), b.end()) {}

    const I & operator * (void) const { return super::pos->right; }
  };
} // namespace bijection

#endif // _BIJECTION_HPP
