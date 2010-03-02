// mirko.rahn@itwm.fraunhofer.de

#ifndef _BITSETOFINT_HPP
#define _BITSETOFINT_HPP

#include <algorithm>
#include <stdexcept>
#include <vector>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>

namespace bitsetofint
{
  template<typename Elem = unsigned int>
  struct type
  {
  private:
    std::vector<unsigned long> _data;

    inline std::size_t _container (const Elem & x) const { return (x >> 6); }
    inline std::size_t _slot (const Elem & x) const { return (x & 63); }

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
      ar & BOOST_SERIALIZATION_NVP(_data);
    }

  public:
    type (const std::size_t num_container = 2) : _data(num_container) {}

    void ins (const Elem & x) throw (std::bad_alloc)
    {
      if (_container(x) >= _data.size())
        _data.resize(_container(x) + 1);
      _data[_container(x)] |= (1UL << _slot(x));
    }

    void del (const Elem & x)
    {
      if (_container(x) < _data.size())
        _data[_container(x)] &= ~(1UL << _slot(x));
    }

    bool is_element (const Elem & x) const
    {
      return (_container(x) < _data.size())
        && ((_data[_container(x)] & (1UL << _slot(x))) != 0);
    }
  };
}

#endif
