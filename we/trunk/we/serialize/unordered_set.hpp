#ifndef BOOST_SERIALIZATION_UNORDEREDSET_HPP
#define BOOST_SERIALIZATION_UNORDEREDSET_HPP

#include <boost/unordered_set.hpp>

#include <boost/config.hpp>

#include <boost/serialization/utility.hpp>
#include <boost/serialization/collections_save_imp.hpp>
#include <boost/serialization/collections_load_imp.hpp>
#include <boost/serialization/split_free.hpp>

namespace boost
{
  namespace serialization
  {
    template<class Archive, class Type, class Compare, class Allocator>
    inline void save
    ( Archive & ar
    , const boost::unordered_set<Type, Compare, Allocator> & t
    , const unsigned int /* file_version */
    )
    {
      boost::serialization::stl::save_collection
        < Archive
        , boost::unordered_set<Type, Compare, Allocator>
        >(ar, t);
    }

    template<class Archive, class Type, class Compare, class Allocator>
    inline void load
    ( Archive & ar
    , boost::unordered_set<Type, Compare, Allocator> & t
    , const unsigned int /* file_version */
    )
    {
      boost::serialization::stl::load_collection
        < Archive
        , boost::unordered_set<Type, Compare, Allocator>
        , boost::serialization::stl::archive_input_set
          < Archive
          , boost::unordered_set<Type, Compare, Allocator> 
          >
        , boost::serialization::stl::no_reserve_imp
          <boost::unordered_set<Type, Compare, Allocator>
          >
        >(ar, t);
    }

    // split non-intrusive serialization function member into separate
    // non intrusive save/load member functions
    template<class Archive, class Type, class Compare, class Allocator>
    inline void serialize
    ( Archive & ar
    , boost::unordered_set<Type, Compare, Allocator> & t
    , const unsigned int file_version
    )
    {
      boost::serialization::split_free(ar, t, file_version);
    }
  } // serialization
} // namespace boost

#endif // BOOST_SERIALIZATION_UNORDEREDSET_HPP
