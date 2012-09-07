#pragma once

namespace fhg
{
  namespace buffer
  {
    template<typename T>
    struct type
    {
    private:
      T* _begin;
      std::size_t _count;
      std::size_t _size;

    public:
      type () : _begin (NULL), _count (0), _size (0) {}
      type (const type& other)
        : _begin (other._begin)
        , _count (other._count)
        , _size (other._size)
      {}
      type& operator = (const type& other)
      {
        _begin = other._begin;
        _count = other._count;
        _size = other._size;
        return *this;
      }
      explicit type (T* begin, const std::size_t size)
        : _begin (begin)
        , _count (size)
        , _size (size)
      {
      }
      T* begin () { return _begin; }
      T* end () { return _begin + _count; }
      const std::size_t& count () const { return _count; }
      std::size_t& count () { return _count; }
      const std::size_t& size () const { return _size; }
    };
  }
}
