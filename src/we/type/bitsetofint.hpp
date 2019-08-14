#pragma once

#include <boost/optional.hpp>
#include <boost/serialization/vector.hpp>

#include <functional>
#include <iosfwd>
#include <set>
#include <vector>

#include <stdint.h>

namespace bitsetofint
{
  struct type
  {
  public:
    explicit type (const std::size_t = 0);

    void push_back (uint64_t);

    type& ins (const unsigned long&);
    type& del (const unsigned long&);
    bool is_element (const unsigned long&) const;
    std::size_t count() const;
    void list (std::ostream&) const;
    void list (const std::function<void (const unsigned long&)>&) const;
    std::set<unsigned long> elements() const;

    friend type operator| (const type&, const type&);
    friend type operator& (const type&, const type&);
    friend type operator^ (const type&, const type&);

    friend std::ostream& operator<< (std::ostream&, const type&);
    friend std::size_t hash_value (const type&);
    friend bool operator== (const type&, const type&);
    friend std::string to_hex (const type&);

    friend bool operator< (const type&, const type&);

    template<typename Archive>
      void serialize (Archive& ar, unsigned int)
    {
      ar & _container;
    }

  private:
    std::vector<uint64_t> _container;
  };

  type operator| (const type&, const type&);
  type operator& (const type&, const type&);
  type operator^ (const type&, const type&);

  bool operator== (const type&, const type&);
  bool operator< (const type&, const type&);

  std::size_t hash_value (const type&);

  std::ostream& operator<< (std::ostream&, const type&);

  std::string to_hex (const type&);
  type from_hex (const std::string&);
  boost::optional<type> from_hex ( std::string::const_iterator& pos
                                 , const std::string::const_iterator& end
                                 );
}
