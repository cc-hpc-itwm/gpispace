// bijection between objects and some index, mirko.rahn@itwm.fraunhofer.de

#ifndef _CONTAINER_BIJECTION_HPP
#define _CONTAINER_BIJECTION_HPP

#include <we/util/it.hpp>

#include <we/container/exception.hpp>

#include <boost/bimap.hpp>
#include <boost/bimap/support/lambda.hpp>
#include <boost/bimap/unordered_set_of.hpp>

#include <boost/serialization/nvp.hpp>

namespace bijection
{
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
    std::string description;

    friend class boost::serialization::access;
    template<class Archive>
    void serialize (Archive & ar, const unsigned int)
    {
      ar & BOOST_SERIALIZATION_NVP(bimap);
      ar & BOOST_SERIALIZATION_NVP(h);
      ar & BOOST_SERIALIZATION_NVP(description);
    }

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

    const I & get_id (const T & x) const
    {
      const typename bimap_t::left_map::const_iterator it (bimap.left.find (x));

      if (it == bimap.left.end())
        throw we::container::exception::no_such (description);

      return it->second;
    }

    const T & get_elem (const I & i) const
    {
      const typename bimap_t::right_map::const_iterator it
        (bimap.right.find (i));

      if (it == bimap.right.end())
        throw we::container::exception::no_such ("index for " + description);

      return it->second;
    }

    const I add (const T & x)
    {
      if (bimap.left.find (x) != bimap.left.end())
        throw we::container::exception::already_there (description);

      const I i (h++);

      bimap.insert (typename bimap_t::value_type (x, i));

      return i;
    }

    void erase (const I & i)
    {
      bimap.right.erase (i);
    }

    I modify (const I & i, const T & x)
    {
      const typename bimap_t::right_map::iterator it (bimap.right.find (i));

      if (it == bimap.right.end())
        throw we::container::exception::no_such ("index for " + description);

      if (bimap.right.modify_data (it, boost::bimaps::_data = x) == false)
        throw we::container::exception::already_there (description + " after modify");

      return i;
    }

    I replace (const I & i, const T & x)
    {
      const typename bimap_t::right_map::iterator it (bimap.right.find (i));

      if (it == bimap.right.end())
        throw we::container::exception::no_such ("index for " + description);

      if (bimap.right.replace_data (it, x) == false)
        throw we::container::exception::already_there (description + " after replace");

      return i;
    }
  };

  template<typename T,typename I>
  struct const_it : public it::it<typename bijection<T, I>::const_iterator>
  {
  public:
    explicit const_it (const bijection<T,I> & b)
      : const_it::super (b.begin(), b.end()) {}

    const I & operator * (void) const { return const_it::super::pos->right; }
  };
} // namespace bijection

#endif
