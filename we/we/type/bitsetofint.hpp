// mirko.rahn@itwm.fraunhofer.de

#ifndef _BITSETOFINT_HPP
#define _BITSETOFINT_HPP

#include <algorithm>
#include <stdexcept>
#include <vector>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>

#include <boost/functional/hash.hpp>

#include <boost/optional.hpp>

#include <iostream>
#include <sstream>
#include <iomanip>

#include <stdint.h>

namespace bitsetofint
{
  typedef unsigned long element_type;
  typedef std::vector<uint64_t> container_type;

  struct type
  {
  public:
    explicit type (const container_type&);
    explicit type (const std::size_t = 0);

    type& ins (const unsigned long&);
    type& del (const unsigned long&);
    bool is_element (const unsigned long&) const;
    std::size_t count() const;
    void list (std::ostream&) const;

    friend type operator| (const type&, const type&);
    friend type operator& (const type&, const type&);
    friend type operator^ (const type&, const type&);

    friend std::ostream& operator<< (std::ostream&, const type&);
    friend std::size_t hash_value (const type&);
    friend bool operator== (const type&, const type&);
    friend std::string to_hex (const type&);

  private:
    container_type _container;

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int)
    {
      ar & BOOST_SERIALIZATION_NVP (_container);
    }
  };

  type operator| (const type&, const type&);
  type operator& (const type&, const type&);
  type operator^ (const type&, const type&);

  bool operator== (const type&, const type&);

  std::size_t hash_value (const type&);

  std::ostream& operator<< (std::ostream&, const type&);

  std::string to_hex (const type&);
  type from_hex (const std::string&);
  boost::optional<type> from_hex ( std::string::const_iterator& pos
                                 , const std::string::const_iterator& end
                                 );
}

#endif
