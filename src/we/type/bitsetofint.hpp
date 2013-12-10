// mirko.rahn@itwm.fraunhofer.de

#ifndef _BITSETOFINT_HPP
#define _BITSETOFINT_HPP

#include <vector>

#include <boost/optional.hpp>
#include <boost/function.hpp>

#include <iosfwd>

#include <stdint.h>
#include <set>

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
    void list (const boost::function<void (const unsigned long&)>&) const;
    std::set<unsigned long> elements() const;

    friend type operator| (const type&, const type&);
    friend type operator& (const type&, const type&);
    friend type operator^ (const type&, const type&);

    friend std::ostream& operator<< (std::ostream&, const type&);
    friend std::size_t hash_value (const type&);
    friend bool operator== (const type&, const type&);
    friend std::string to_hex (const type&);

    friend bool operator< (const type&, const type&);

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

#endif
